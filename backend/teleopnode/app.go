package teleopnode

import (
	"context"
	"errors"
	"fmt"
	"log"
	"net/http"
	"sort"
	"strconv"
	"sync"
	"time"

	jwt "github.com/dgrijalva/jwt-go"
	assetfs "github.com/elazarl/go-bindata-assetfs"
	"github.com/golang/protobuf/proto"
	"github.com/gorilla/mux"
	"github.com/gorilla/websocket"
	"github.com/zippyai/zippy/backend/configs"
	"github.com/zippyai/zippy/backend/data"
	"github.com/zippyai/zippy/backend/plainui"
	"github.com/zippyai/zippy/backend/services"
	"github.com/zippyai/zippy/backend/teleop"
	"github.com/zippyai/zippy/backend/teleopui"
	calibration "github.com/zippyai/zippy/packages/calibration/proto"
	core "github.com/zippyai/zippy/packages/core/proto"
	halproto "github.com/zippyai/zippy/packages/hal/proto"
	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

// this anonymous function is only ever implemented by the test library to give us
// a blocking channel hook into the code in order to combat race conditions in testing
var (
	done = func() {}
)

const (
	// buffer size for channel for sending messages to vehicle
	vehicleQueue        = 3
	abnormalWSCloseCode = 1006
)

// setup some websocket parameters
var upgrader = websocket.Upgrader{
	ReadBufferSize:    1 << 20, // 1MB
	WriteBufferSize:   1 << 20, // 1MB
	EnableCompression: true,
	CheckOrigin: func(r *http.Request) bool {
		return true
	},
}

// NewApp generate an app state from the config options
func NewApp(options *Options) (*App, error) {

	callbackHost := options.configs.Domain
	if callbackHost == "localhost" {
		callbackIP, err := services.GetPrivateIP()
		if err != nil {
			return nil, err
		}

		callbackHost = fmt.Sprintf("%s:%d", callbackIP, options.configs.Port)
	}

	simRunner := services.SetUpSimulatorRunner(callbackHost, options.configs.SSHConfig.Username, options.configs.SSHConfig.KeyFile, options.configs.SSHConfig.SimServerAddr)

	userNotifier := services.NewInternalManagedNotifier()

	return &App{
		relays:             make(map[string]*relay),
		simulatorListeners: make(map[string]chan error),
		storage:            options.storage,
		env:                options.env,
		authRealm:          options.authRealm,
		assets:             options.assets,
		config:             options.configs,
		secrets:            options.secrets,
		simController:      simRunner,
		userNotifier:       userNotifier,
	}, nil
}

// App encapsulates the top-level server state
type App struct {
	mu                 sync.Mutex
	storage            data.Storage
	authRealm          string
	assets             Assets
	env                configs.Environment
	secrets            *configs.Secrets
	config             *configs.Config
	relays             map[string]*relay
	simulatorListeners map[string]chan error
	simController      services.SimulatorController
	userNotifier       services.ManagedNotifier
}

// Assets contains the filesystem locations for the apps assets
type Assets struct {
	TeleopAssets http.FileSystem
	PlainAssets  http.FileSystem
}

func generateAssets(frontend, plain string) Assets {
	assets := Assets{}
	if frontend != "" {
		assets.TeleopAssets = http.Dir(frontend)
	} else {
		assets.TeleopAssets = &assetfs.AssetFS{
			Asset:     teleopui.Asset,
			AssetDir:  teleopui.AssetDir,
			AssetInfo: teleopui.AssetInfo,
		}
	}

	if plain != "" {
		assets.PlainAssets = http.Dir(plain)
	} else {
		assets.PlainAssets = &assetfs.AssetFS{
			Asset:     plainui.Asset,
			AssetDir:  plainui.AssetDir,
			AssetInfo: plainui.AssetInfo,
		}
	}

	return assets
}

// relay gets the relay for the given vehicle ID
func (a *App) relay(id string) (*relay, bool) {
	a.mu.Lock()
	defer a.mu.Unlock()

	r, found := a.relays[id]
	return r, found
}

// relayOrCreate get the relay for the given vehicle ID, or creates on
func (a *App) relayOrCreate(id string) *relay {
	a.mu.Lock()
	defer a.mu.Unlock()

	r, found := a.relays[id]
	if !found {
		r = &relay{
			id:        id,
			listeners: make(map[listenerType]map[string]*vehicleListener),
		}
		a.relays[id] = r
	}
	return r
}

// the handler for the websocket endpoint
func (a *App) handleVehicleWS(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	vid := vars["vid"]
	log.Println("at handleVehicleWS:", vid)
	defer log.Println("returning from handleVehicleWS")
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		return
	}

	sessionID := vid
	sessionCookie, err := r.Cookie(services.VehicleSessionCookieName)
	if err == nil {
		log.Println("vehicle session cookie present for vehicle: ", vid)
		sessionID = sessionCookie.Value
	}
	defer func(now time.Time) {
		websocketLifeSpan.With("endpoint", pathTmpl, "requester", sessionID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	// upgrade from ordinary http request to websocket
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("error upgrading to websocket connection:", err)
		return
	}
	conn.EnableWriteCompression(true)
	openWebsockets.With("endpoint", pathTmpl, "requester", sessionID).Add(1)
	defer openWebsockets.With("endpoint", pathTmpl, "requester", sessionID).Add(-1)

	// get the relay or create it
	relay := a.relayOrCreate(sessionID)
	log.Println("creating new session information for session id: ", sessionID)
	err = services.RegisterVehicle(a.storage, sessionID, vid)
	if err != nil {
		log.Println("error registering vehicle on connection: ", err)
		return
	}

	// create cancellation context
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	// create channel for messages to the vehicle
	outgoing := make(chan *teleopproto.BackendMessage, vehicleQueue)

	// set this connection as the websocket
	relay.setVehicle(conn, outgoing, cancel)
	defer relay.clearVehicle() // TODO: this may clear the new vehicle on replacement

	// read from the connection
	fromVehicle := make(chan []byte)
	errorChan := make(chan error)
	go func(errorChan chan error) {
		if err := readMessages(ctx, fromVehicle, conn); err != nil {
			errorChan <- err
		}
	}(errorChan)

	// for testing this is implemented to let them know that this concurrent process is done
	// and that the websocket conn setup is now done
	done()

	gotManifest := false
	// notify any listening users that the simulator is ready
	a.simController.Notify(sessionID, nil)
	// session state change that happened in loop
	sessionState := data.StatusActive
	// loop until client hangs up
OuterLoop:
	for {
		select {
		case err = <-errorChan:
			log.Println("error reading from websocket: ", err)
			sessionState = data.StatusOffline
			if websocket.IsCloseError(err, abnormalWSCloseCode) { // check for an abnormal error code meaning network dropped
				sessionState = data.StatusNetworkDrop
			}

			break OuterLoop

		case <-ctx.Done():
			log.Println("context is done, breaking out of handleVehicleWS")
			break OuterLoop

		case buf := <-fromVehicle:
			if buf == nil {
				log.Println("channel closed, breaking out of handleVehicleWS")
				sessionState = data.StatusOffline
				break OuterLoop
			}

			log.Printf("read %d bytes from vehicle", len(buf))

			// parse protobuf
			var msg teleopproto.VehicleMessage
			err = proto.Unmarshal(buf, &msg)
			if err != nil {
				log.Println("error unmarshalling message from vehicle:", err)
				return
			}

			log.Printf("got update: %T", msg.GetPayload())
			websocketFramesSent.With("endpoint", pathTmpl, "requester", sessionID).Add(1)

			// update the latest location
			var msgType listenerType
			switch p := msg.GetPayload().(type) {
			case *teleopproto.VehicleMessage_Frame:
				if p.Frame == nil {
					log.Println("payload from vehicle had type camera sample, but frame was empty")
					continue
				}

				// this is a temp fix in order to make up for c++ not sending device info
				// once we have a  device sending up we should drop frame if nil instead
				if !gotManifest {
					log.Println("had to write dummy vehicle manifest since one wasn't sent")
					a.saveManifest(vid, makeDummyCamera())
					if err != nil {
						log.Printf("unable to store latest frame error: %s\n", err)
					} else {
						gotManifest = true
					}
				}

				sampleView := teleop.CameraSampleFromProto(p.Frame)
				if sampleView == nil {
					continue
				}

				// store camera frame non blocking best effort
				go func(name string) {
					err := services.SaveNewCameraFrame(a.storage, vid, sampleView)
					if err != nil {
						log.Printf("unable to store latest frame from camera: %s error: %s\n", name, err)
					}
				}(sampleView.Camera)

				// only notify camera subscribers
				msgType = camera
				log.Println("got frame from vehicle with id: ", vid)
			case *teleopproto.VehicleMessage_Gps:
				locationView := teleop.LocationSampleFromProto(p.Gps)
				err := services.UpdateLocation(a.storage, sessionID, locationView)
				if err != nil {
					log.Println("unable to update location information in db error: ", err)
				}

				// only notify location subscribers
				msgType = location
				log.Println("got new vehicle gps location for vehicleID: ", vid)
			case *teleopproto.VehicleMessage_Manifest:
				a.saveManifest(vid, p.Manifest.Cameras...)
				if err != nil {
					log.Printf("unable to store vehicle manifest error: %s\n", err)
					continue
				}

				gotManifest = true
				// do not notify anyone just drop
				msgType = drop
				log.Printf("received manifest with %d cameras", len(p.Manifest.Cameras))
			case *teleopproto.VehicleMessage_DockingObservation:
				// only notify vehicle controller subscribers
				msgType = control
				log.Printf("observed %d docking stations at %s\n", len(p.DockingObservation.StationIds), p.DockingObservation.Timestamp.String())
			case *teleopproto.VehicleMessage_DockingStatus:
				// only notify vehicle controller subscribers
				msgType = control
				log.Printf("received docking status update %s distanceX remaining: %f distanceY remaining: %f angle remaining: %f\n", p.DockingStatus.Status.String(), p.DockingStatus.RemainingDistanceX, p.DockingStatus.RemainingDistanceY, p.DockingStatus.RemainingAngle)
			case *teleopproto.VehicleMessage_Confirmation:
				// only notify vehicle controller subscribers
				msgType = control
			case *teleopproto.VehicleMessage_Detection,
				*teleopproto.VehicleMessage_Detection3D:
				// send the different 3d detections the vehicle has seen to the frontend
				msgType = view
			case *teleopproto.VehicleMessage_VehicleStatus:
				// send status messages from the vehicle to the viewer web-socket
				msgType = view
			case *teleopproto.VehicleMessage_SdpRequest,
				*teleopproto.VehicleMessage_SdpComfirmation,
				*teleopproto.VehicleMessage_IceCandidate:
				// relay all webrtc messages to the webrtc websocket
				msgType = webrtc
			}

			// for testing this is implemented to let them know that this concurrent process is done
			// and that the values set here are now safe to access
			done()

			// broadcast to listeners
			relay.notifyType(msgType, &msg)

		case msg := <-outgoing:
			// send message out
			duration := timingManager.Finish(msg.Id)
			err = SendProto(conn, msg)
			// record the time it took to proccess the command
			commandProcessTime.With("recipient", sessionID, "type", messageType(msg)).Observe(duration.Seconds() * 1e3)
			if err != nil {
				log.Println("error writing to vehicle:", err)
				sessionState = data.StatusNetworkDrop
				break OuterLoop
			}

			// for testing this is implemented to let them know that this concurrent process is done
			// and that it is now safe to read a message sent from the vehicle to the client
			done()
		}
	}

	// notify the controller that the vehicle websocket is dead
	controllerID, err := services.UserInControl(a.storage, sessionID)
	if err != nil {
		log.Println("unable to get controllers id from session: ", err)
	}
	a.userNotifier.Notify(controllerID, teleop.NewPGDeadNotification(sessionID))

	// let all client websocket threads know that nothing more is coming over the listener now so they can die
	relay.notifyVehicleStateChange(sessionState)
	log.Printf("vehicle with id: %s is disconnecting for sessionID: %s\n", vid, sessionID)
	err = services.UpdateSessionStatus(a.storage, sessionID, sessionState)
	if err != nil {
		log.Println("error in updating the vehicles session state in redis")
	}
}

// tmp func to ensure the creation of a vehicle and the saving of it's manifest
func (a *App) saveManifest(vehicleID string, cameras ...*teleopproto.Camera) error {
	numSaved, err := services.SaveNewCameras(a.storage, vehicleID, cameras...)
	if err != nil {
		return err
	}

	if numSaved < len(cameras) {
		log.Println("some cameras were dropped because no device was provided")
	}

	// best effort of creation, but if the vehicle record already exists this is not a failure
	a.storage.CreateVehicle(vehicleID)

	return nil
}

func durationOrZero(s string) (time.Duration, error) {
	if s == "" {
		return 0, nil
	}
	return time.ParseDuration(s)
}

func (a *App) handleSubscribeCamera(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	sessionID := vars["sid"]
	cam := vars["cam"]
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	if !services.VehicleActive(a.storage, sessionID) {
		err := fmt.Errorf("vehicle session with id: %s is not active or doesn't exist", sessionID)
		log.Println(err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
	}

	requesterID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer func(now time.Time) {
		websocketLifeSpan.With("endpoint", pathTmpl, "requester", sessionID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	log.Println("at handleSubscribeCamera:", sessionID)
	defer log.Println("returning from handleSubscribeCamera")

	// upgrade from ordinary http request to websocket
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("error upgrading to websocket connection:", err)
		return
	}
	defer conn.Close()
	openWebsockets.With("endpoint", pathTmpl, "requester", requesterID).Add(1)
	defer openWebsockets.With("endpoint", pathTmpl, "requester", requesterID).Add(-1)

	// get the relay or create it
	relay := a.relayOrCreate(sessionID)

	// create cancellation context
	ctx, cancel := context.WithCancel(context.Background())

	// register listener
	listener := relay.listen(camera, cancel)
	defer relay.unlisten(listener.id)

	// for testing this is implemented to let them know that this concurrent process is done
	// and that the websocket conn settup is now done
	done()

	// loop until the client hangs up
OuterLoop:
	for {
		select {
		case <-ctx.Done():
			log.Println("context is done, breaking out of handleSubscribeCamera")
			break OuterLoop

		case msg := <-listener.ch:
			pl, ok := msg.GetPayload().(*teleopproto.VehicleMessage_Frame)
			if !ok {
				continue
			}

			websocketFramesSent.With("endpoint", pathTmpl, "requester", requesterID).Add(1)
			if cam != "all" && pl.Frame.GetDevice().GetName() != cam {
				continue
			}

			err := SendJSON(conn, teleop.CameraSampleFromProto(pl.Frame))
			if err != nil {
				log.Println("error writing frame to subscriber:", err)
				return
			}
		}
	}
}

func (a *App) handleListVehicles(w http.ResponseWriter, r *http.Request) {
	queryValues := r.URL.Query()
	stateStr := queryValues.Get("state")
	limitStr := queryValues.Get("limit")
	offsetStr := queryValues.Get("offset")
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	requesterID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer func(now time.Time) {
		httpRequestTime.With("endpoint", pathTmpl, "requester", requesterID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var limit int
	var offset int
	if limitStr != "" {
		limit, err = strconv.Atoi(limitStr)
		if err != nil {
			log.Printf("unable to convert limit %s to int error: %s", queryValues.Get("limit"), err)
			respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
			return
		}
	}

	if offsetStr != "" {
		offset, err = strconv.Atoi(offsetStr)
		if err != nil {
			log.Printf("unable to convert limit %s to int error: %s", queryValues.Get("offset"), err)
			respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
			return
		}
	}

	if limit <= 0 {
		limit = len(a.relays)
	}

	var status *data.VehicleStatus
	if stateStr != "" {
		fmt.Println("state string we got: ", stateStr)
		statusp := new(data.VehicleStatus)
		status = statusp
		if err := status.UnmarshalText([]byte(stateStr)); err != nil {
			log.Printf("unable to convert status: %s to VehicleStatus error: %s", stateStr, err)
			respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
			return
		}
	}

	var i int
	vehicles := make([]*teleop.VehicleView, 0) // avoid null in json
	for _, relay := range a.relays {
		if i < offset {
			continue
		}

		if i >= limit {
			break
		}

		vehicleView, err := services.GetVehicleView(a.storage, relay.id, requesterID, status)
		if err != nil {
			if err != data.ErrNoVehicle && err != services.ErrWrongVehicleStatus {
				log.Printf("unable to pull vehicle with id %s from storage error: %s", relay.id, err)
				respondError(w, r, err, requesterID, pathTmpl, http.StatusInternalServerError)
				return
			}

			continue
		}

		i++
		vehicles = append(vehicles, vehicleView)
	}

	// sort vehicles for stability
	sort.Slice(vehicles, func(a, b int) bool {
		return vehicles[a].ID < vehicles[b].ID
	})

	respondJSON(w, r, requesterID, map[string]interface{}{
		"vehicles": vehicles,
	})
}

func (a *App) handleVehicle(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	sessionID := vars["sid"]
	queryValues := r.URL.Query()
	stateStr := queryValues.Get("state")
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	requesterID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer func(now time.Time) {
		httpRequestTime.With("endpoint", pathTmpl, "requester", requesterID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var status *data.VehicleStatus
	if stateStr != "" {
		if err := status.UnmarshalText([]byte(stateStr)); err != nil {
			log.Printf("unable to convert status: %s to VehicleStatus error: %s", stateStr, err)
			respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
			return
		}
	}

	vehicleView, err := services.GetVehicleView(a.storage, sessionID, requesterID, status)
	if err != nil {
		log.Printf("unable to pull vehicle with id %s from storage error: %s", sessionID, err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusInternalServerError)
		return
	}

	respondJSON(w, r, requesterID, map[string]interface{}{
		"vehicle": vehicleView,
	})
}

const (
	lastFrameKey = "latest"
	limitKey     = "limit"
	sinceKey     = "since"
)

func (a *App) getFrames(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	vid := vars["vid"]
	cameraID := vars["cid"]
	params := r.URL.Query()
	latest := params.Get(lastFrameKey)
	limitStr := params.Get(limitKey)
	sinceStr := params.Get(sinceKey)
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	requesterID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer func(now time.Time) {
		httpRequestTime.With("endpoint", pathTmpl, "requester", requesterID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	limit := 1000
	if limitStr != "" {
		limit, err = strconv.Atoi(limitStr)
		if err != nil {
			log.Println("requested limit is not parsable as an int: ", err)
			respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
			return
		}
	}

	var since time.Time
	if sinceStr != "" {
		sinceUnix, err := strconv.Atoi(sinceStr)
		if err != nil {
			log.Println("the requested timestamp is not parsable: ", err)
			respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
			return
		}

		since = time.Unix(0, int64(sinceUnix))
	}

	var frames interface{}
	if latest == "true" {
		frames, err = services.GetLastFrame(a.storage, vid, cameraID)
		if err != nil {
			if err == data.ErrNoCamera || err == data.ErrNoVehicle {
				log.Printf("the vehicle or camera you specified does not exist vehicleID: %s cameraID: %s error: %s\n", vid, cameraID, err)
				respondError(w, r, err, requesterID, pathTmpl, http.StatusNotFound)
				return
			}

			log.Println("unable to get the last frame for the camera: ", err)
			respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
			return
		}
	} else if !since.IsZero() {
		frames, err = services.CameraFramesSince(a.storage, vid, cameraID, since, limit)
		if err != nil {
			fmt.Println(err)
			if err == data.ErrNoCamera || err == data.ErrNoVehicle {
				log.Printf("the vehicle or camera you specified does not exist vehicleID: %s cameraID: %s error: %s\n", vid, cameraID, err)
				respondError(w, r, err, requesterID, pathTmpl, http.StatusNotFound)
				return
			}

			log.Printf("unable to get the frames since %s for the camera error: %s\n", since.String(), err)
			respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
			return
		}
	} else {
		frames, err = services.LimitedCameraFrames(a.storage, vid, cameraID, limit)
		if err != nil {
			if err == data.ErrNoCamera || err == data.ErrNoVehicle {
				log.Printf("the vehicle or camera you specified does not exist vehicleID: %s cameraID: %s error: %s\n", vid, cameraID, err)
				respondError(w, r, err, requesterID, pathTmpl, http.StatusNotFound)
				return
			}

			log.Printf("unable to get the last %d frames for the camera with id: %s error: %s\n", limit, cameraID, err)
			respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
			return
		}
	}

	respondJSON(w, r, requesterID, frames)
}

func userIDFromRequest(req *http.Request) (string, error) {
	claims, ok := req.Context().Value(jwtClaimsKey).(jwt.MapClaims)
	if !ok {
		return "", errors.New("unable to get token claims from request")
	}

	id, ok := claims["identifier"]
	if !ok {
		return "", errors.New("user identifier is not present in claims")
	}

	userID, ok := id.(string)
	if !ok {
		return "", errors.New("user identifier is not a string in claims")
	}

	return userID, nil
}

func makeDummyCamera() *teleopproto.Camera {
	return &teleopproto.Camera{
		Role:   halproto.CameraId_FrontFisheye,
		Height: 600,
		Width:  1200,
		Device: &halproto.Device{
			Name:         "front",
			SerialNumber: 123,
		},
		// adding intrinsics and extrinsics so that the front end can check the format
		Intrinsics: &calibration.CameraIntrinsicCalibration{
			CameraUnderCalibration: &halproto.Device{
				Name:         "front",
				SerialNumber: 123,
			},
			OpticalCenterX:     0.0,
			OpticalCenterY:     0.0,
			ResolutionX:        0,
			ResolutionY:        0,
			ScaledFocalLengthX: 0,
			ScaledFocalLengthY: 0,
			Skew:               0.0,
			Distortion: &calibration.CameraIntrinsicCalibration_KannalaBrandt{
				KannalaBrandt: &calibration.KannalaBrandtDistortionModel{
					RadialDistortionCoefficientI1:     0,
					RadialDistortionCoefficientI2:     0,
					RadialDistortionCoefficientI3:     0,
					RadialDistortionCoefficientI4:     0,
					RadialDistortionCoefficientK:      []float64{0},
					RadialDistortionCoefficientL1:     0,
					RadialDistortionCoefficientL2:     0,
					RadialDistortionCoefficientL3:     0,
					TangentialDistortionCoefficientJ1: 0,
					TangentialDistortionCoefficientJ2: 0,
					TangentialDistortionCoefficientJ3: 0,
					TangentialDistortionCoefficientJ4: 0,
					TangentialDistortionCoefficientM1: 0,
					TangentialDistortionCoefficientM2: 0,
					TangentialDistortionCoefficientM3: 0,
				},
			},
		},
		Extrinsics: &calibration.CoordinateTransformation{
			SourceCoordinateFrame: &calibration.CoordinateFrame{
				Anchor: &calibration.CoordinateFrame_Device{
					Device: &halproto.Device{
						Name:         "front",
						SerialNumber: 123,
					},
				},
				ValidPeriodBegin: &core.SystemTimestamp{
					Nanos: 0,
				},
				ValidPeriodEnd: &core.SystemTimestamp{
					Nanos: 0,
				},
			},
			TargetCoordinateFrame: &calibration.CoordinateFrame{
				Anchor: &calibration.CoordinateFrame_Device{
					Device: &halproto.Device{
						Name:         "front",
						SerialNumber: 123,
					},
				},
				ValidPeriodBegin: &core.SystemTimestamp{
					Nanos: 0,
				},
				ValidPeriodEnd: &core.SystemTimestamp{
					Nanos: 0,
				},
			},
			RodriguesRotationX:    0,
			RodriguesRotationY:    0,
			RodriguesRotationZ:    0,
			TimeOffsetNanoseconds: 0,
			TranslationX:          0,
			TranslationY:          0,
			TranslationZ:          0,
		},
	}
}
