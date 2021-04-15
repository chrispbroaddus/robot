package teleopnode

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"net/http/httptest"
	"net/url"
	"os"
	"testing"
	"time"

	"github.com/adams-sarah/test2doc/test"
	"github.com/golang/protobuf/proto"
	"github.com/gorilla/mux"
	"github.com/gorilla/websocket"
	uuid "github.com/satori/go.uuid"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"github.com/zippyai/zippy/backend/configs"
	"github.com/zippyai/zippy/backend/data"
	"github.com/zippyai/zippy/backend/services"
	"github.com/zippyai/zippy/backend/teleop"
	calibration "github.com/zippyai/zippy/packages/calibration/proto"
	hal1 "github.com/zippyai/zippy/packages/hal/proto"
	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

var (
	app         *App
	server      *test.Server
	router      *mux.Router
	wsServer    *httptest.Server
	client      http.Client
	adminCookie *http.Cookie
	authHeader  http.Header
	badCookie   *http.Cookie
	badHeader   http.Header
)

func TestMain(m *testing.M) {
	var err error
	app, err = CreateTestApp(configs.Local)
	if err != nil {
		panic("unable to create a local test app")
	}

	router = app.CreateRouter()
	fmt.Println(router.Get("register"))
	test.RegisterURLVarExtractor(mux.Vars)
	client = http.Client{}

	server, err = test.NewServer(router)
	if err != nil {
		panic(fmt.Sprintf("error while starting test server: %s", err))
	}

	wsServer = httptest.NewServer(router)
	defer wsServer.Close()

	adminCookie, err = loginUser(server.URL, &teleop.UserAuth{
		UserName: "admin",
		Password: "admin",
	})
	if err != nil {
		panic(fmt.Sprintf("error while attempting to authenticate with default user: %s", err))
	}

	authHeader = http.Header{}
	authHeader.Add("Cookie", adminCookie.String())

	badCookie = &http.Cookie{
		Name:  "auth",
		Value: "not_real",
	}

	badHeader = http.Header{}
	badHeader.Add("Cookie", badCookie.String())

	log.Println("starting tests")
	exitCode := m.Run()

	fmt.Println("server finished and cleaning up now")
	server.Finish()

	os.Exit(exitCode)
}

// CreateTestApp creates the top-level application state for testing
func CreateTestApp(env configs.Environment) (*App, error) {
	opts := &Options{}
	storage, err := opts.newStorage()
	if err != nil {
		return nil, err
	}

	return &App{
		relays:        make(map[string]*relay),
		storage:       storage,
		env:           env,
		secrets:       &configs.Secrets{TokenSigningKey: []byte("test signing")},
		simController: &mockSimController{runningSims: make(map[string]*testMessanger)},
		userNotifier:  services.NewInternalManagedNotifier(),
	}, nil
}

const (
	clientID = "u01"

	// base api uri
	baseAPIURI = "/api/v1"
)

func makeFrame(tag, cam string) *teleopproto.VehicleMessage {
	const (
		width  = 10
		height = 20
	)
	buf := make([]byte, width*height)

	tagbuf, _ := json.Marshal(tag)
	copy(buf, tagbuf)
	jpeg := teleop.NewJPEG(buf, width, height)
	jpeg.Device = teleop.NewDevice(cam)
	return teleop.NewFrameMessage(jpeg)
}

func makeGPS(lat, lon float64) *teleopproto.VehicleMessage {
	return teleop.NewGPSMessage(teleop.NewGPSTelemetry(lat, lon))
}

func tag(b []byte) string {
	var x string
	err := json.NewDecoder(bytes.NewReader(b)).Decode(&x)
	if err != nil {
		panic(err)
	}
	return x
}

func dialWS(baseURL, path string, header http.Header) (*websocket.Conn, error) {
	wsurl, err := url.Parse(baseURL)
	if err != nil {
		return nil, err
	}

	wsurl.Scheme = "ws"
	wsurl, err = wsurl.Parse(path)
	if err != nil {
		return nil, err
	}

	var dialer websocket.Dialer
	conn, _, err := dialer.Dial(wsurl.String(), header)
	return conn, err
}

func RecvVehicleMsg(conn *websocket.Conn) (*teleopproto.VehicleMessage, error) {
	var msg teleopproto.VehicleMessage
	return &msg, RecvJSONPB(conn, &msg)
}

func httpGet(url string, dest interface{}) error {
	resp, err := http.Get(url)
	if err != nil {
		return err
	}

	buf, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return err
	}

	return json.Unmarshal(buf, dest)
}

func newSessionCookie(vehicleID string) (*http.Cookie, error) {
	tokenGenURL := fmt.Sprintf("/api/v1/vehicle/generate?vehicle_id=%s", vehicleID)
	req, err := makeReq(server.URL, tokenGenURL, "POST", nil, adminCookie)
	if err != nil {
		return nil, err
	}

	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	auth := new(teleop.VehicleAuthView)
	err = json.NewDecoder(resp.Body).Decode(auth)
	if err != nil {
		return nil, err
	}

	// validate the token we just generated
	validateTokenURL := fmt.Sprintf("/api/v1/vehicle/token/%s/validate", vehicleID)
	req, err = makeReq(server.URL, validateTokenURL, "POST", nil, adminCookie)
	if err != nil {
		return nil, err
	}

	resp, err = client.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	sessionURL := "/api/v1/vehicle/auth"
	req, err = makeReq(server.URL, sessionURL, "POST", auth, nil)
	if err != nil {
		return nil, err
	}

	resp, err = client.Do(req)
	if err != nil {
		return nil, err
	}

	for _, cookie := range resp.Cookies() {
		if cookie.Name == services.VehicleSessionCookieName {
			return cookie, nil
		}
	}

	return nil, fmt.Errorf("cookie not returned")
}

// this is used to implament the anonymous done function used in the code.
// this implamentation will fire off on this channel which we'll catch in the tests
// in order to force synchronicity between the server and the test threads
func setDoneListener() chan struct{} {
	doneChan := make(chan struct{})
	done = func() {
		doneChan <- struct{}{}
	}

	return doneChan
}

func TestApp_SingleVehicleMessage(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()

	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())
	vehicleConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID), sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	cameraID := "front"
	operatorConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/camera/%s/subscribe", sessionCookie.Value, cameraID), authHeader)
	require.NoError(t, err)
	defer operatorConn.Close()
	<-doneChan

	orig := makeFrame("orig", cameraID)

	err = SendProto(vehicleConn, orig)
	require.NoError(t, err)
	<-doneChan
	<-doneChan

	sample := new(teleop.CameraSample)
	err = RecvJSON(operatorConn, sample)
	require.NoError(t, err)
	assert.Contains(t, string(sample.Image.Content), "orig")
}

// this test checks what happens when there are multiple listeners
func TestApp_MultipleOperators(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()

	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())
	vehicleConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID), sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	cameraID := "front"
	operatorConn1, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/camera/%s/subscribe", sessionCookie.Value, cameraID), authHeader)
	require.NoError(t, err)
	defer operatorConn1.Close()
	<-doneChan

	operatorConn2, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/camera/%s/subscribe", sessionCookie.Value, cameraID), authHeader)
	require.NoError(t, err)
	defer operatorConn2.Close()
	<-doneChan

	orig := makeFrame("orig", cameraID)

	err = SendProto(vehicleConn, orig)
	require.NoError(t, err)
	<-doneChan
	<-doneChan
	<-doneChan

	sample := new(teleop.CameraSample)
	err = RecvJSON(operatorConn1, sample)
	require.NoError(t, err)
	assert.Contains(t, string(sample.Image.Content), "orig")

	err = RecvJSON(operatorConn2, sample)
	require.NoError(t, err)
	assert.Contains(t, string(sample.Image.Content), "orig")
}

func TestApp_ListVehicles(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()

	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())
	vehicleConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID), sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	var resp *teleop.VehiclesView

	baseURL := "/api/v1/vehicles"
	req, err := makeReq(server.URL, baseURL, "GET", nil, adminCookie)
	require.NoError(t, err)
	reply, err := client.Do(req)
	require.NoError(t, err)
	defer reply.Body.Close()
	err = json.NewDecoder(reply.Body).Decode(&resp)
	require.NoError(t, err)
	require.True(t, len(resp.Vehicles) >= 1)
	vehicle := getVehicleByName(vehicleID, resp.Vehicles)
	assert.Equal(t, data.StatusActive, vehicle.Status)
	resp = nil

	// only grab those with an active state
	desiredState, err := data.StatusActive.MarshalText()
	require.NoError(t, err)
	req, err = makeReq(server.URL, fmt.Sprintf("%s?state=%s", baseURL, string(desiredState)), "GET", nil, adminCookie)
	require.NoError(t, err)
	reply, err = client.Do(req)
	require.NoError(t, err)
	defer reply.Body.Close()
	err = json.NewDecoder(reply.Body).Decode(&resp)
	require.NoError(t, err)
	require.True(t, len(resp.Vehicles) >= 1)
	for _, vehicle := range resp.Vehicles {
		require.Equal(t, vehicle.Status, data.StatusActive)
	}
	resp = nil

	//	set user as a viewer
	err = services.AddVehicleViewer(app.storage, sessionCookie.Value, "u01")
	require.NoError(t, err)
	// ensure that we grab viewer info if we request it
	req, err = makeReq(server.URL, fmt.Sprintf("%s?viewers=true", baseURL), "GET", nil, adminCookie)
	require.NoError(t, err)
	reply, err = client.Do(req)
	require.NoError(t, err)
	defer reply.Body.Close()
	err = json.NewDecoder(reply.Body).Decode(&resp)
	require.NoError(t, err)
	vehicle = getVehicleByName(vehicleID, resp.Vehicles)
	require.NotNil(t, vehicle)
	require.NotNil(t, vehicle.Viewers)
}

func TestApp_GetVehicleFailure(t *testing.T) {
	vehicleID := uuid.NewV4().String()
	baseURL := "/api/v1/vehicle"
	// request with bad auth information
	req, err := makeReq(server.URL, fmt.Sprintf("%s/%s", baseURL, vehicleID), "GET", nil, badCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusUnauthorized, resp.StatusCode)

	// request with bad state as string failure
	req, err = makeReq(server.URL, fmt.Sprintf("%s/%s?state=%s", baseURL, vehicleID, "bad_state"), "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusBadRequest, resp.StatusCode)

	// request for a vehicle that doesn't exist
	req, err = makeReq(server.URL, fmt.Sprintf("%s/%s", baseURL, vehicleID), "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusInternalServerError, resp.StatusCode)

}

func TestApp_ListVehiclesFailure(t *testing.T) {
	baseURL := "/api/v1/vehicles"
	// request with bad auth information
	req, err := makeReq(server.URL, baseURL, "GET", nil, badCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusUnauthorized, resp.StatusCode)

	// request with limit as string failure
	req, err = makeReq(server.URL, fmt.Sprintf("%s?limit=%s", baseURL, "bad_limit"), "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusBadRequest, resp.StatusCode)

	// request with offset as string failure
	req, err = makeReq(server.URL, fmt.Sprintf("%s?limit=1&offset=%s", baseURL, "bad_offset"), "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusBadRequest, resp.StatusCode)

	// request with bad state as string failure
	req, err = makeReq(server.URL, fmt.Sprintf("%s?state=%s", baseURL, "bad_state"), "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusBadRequest, resp.StatusCode)
}

func getVehicleByName(vehicleID string, vehicles []*teleop.VehicleView) *teleop.VehicleView {
	for _, vehicle := range vehicles {
		if vehicle.VehicleID == vehicleID {
			return vehicle
		}
	}

	return nil
}

func TestApp_ListTwoVehicles(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()
	vehicleID2 := uuid.NewV4().String()

	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())
	vehicleConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID), sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	sessionCookie2, err := newSessionCookie(vehicleID2)
	require.NoError(t, err)
	sessionHeader2 := http.Header{}
	sessionHeader2.Add("Cookie", sessionCookie2.String())
	conn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID2), sessionHeader2)
	require.NoError(t, err)
	defer conn.Close()
	<-doneChan

	var resp *teleop.VehiclesView
	req, err := makeReq(server.URL, "api/v1/vehicles", "GET", nil, adminCookie)
	require.NoError(t, err)
	reply, err := client.Do(req)
	require.NoError(t, err)
	defer reply.Body.Close()
	blob, err := ioutil.ReadAll(reply.Body)
	require.NoError(t, err)
	err = json.Unmarshal(blob, &resp)
	require.NoError(t, err)
	// require.Len(t, resp.Vehicles, 2)

	v1 := getVehicleByName(vehicleID, resp.Vehicles)
	assert.Equal(t, data.StatusActive, v1.Status)

	v2 := getVehicleByName(vehicleID2, resp.Vehicles)
	assert.Equal(t, data.StatusActive, v2.Status)

	baseURL := "/api/v1/vehicles"
	// only grab those with an active state
	desiredState, err := data.StatusActive.MarshalText()
	require.NoError(t, err)
	req, err = makeReq(server.URL, fmt.Sprintf("%s?state=%s&limit=1", baseURL, string(desiredState)), "GET", nil, adminCookie)
	require.NoError(t, err)
	reply, err = client.Do(req)
	require.NoError(t, err)
	defer reply.Body.Close()
	err = json.NewDecoder(reply.Body).Decode(&resp)
	require.NoError(t, err)
	require.True(t, len(resp.Vehicles) == 1)
	for _, vehicle := range resp.Vehicles {
		require.Equal(t, vehicle.Status, data.StatusActive)
	}
	resp = nil
}

func TestApp_GetVehicle(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()

	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())
	vehicleConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID), sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	msg0 := teleop.NewManifestMessage(&teleopproto.Manifest{
		Cameras: []*teleopproto.Camera{
			&teleopproto.Camera{
				Device: teleop.NewDevice("forward"),
				Role:   hal1.CameraId_FrontFisheye,
				Width:  640,
				Height: 360,
				Intrinsics: &calibration.CameraIntrinsicCalibration{
					OpticalCenterX: 180,
					OpticalCenterY: 320,
				},
				Extrinsics: &calibration.CoordinateTransformation{
					RodriguesRotationX: 1,
				},
			},
			&teleopproto.Camera{
				Device: teleop.NewDevice("backward"),
				Role:   hal1.CameraId_RearFisheye,
				Width:  1080,
				Height: 720,
				Intrinsics: &calibration.CameraIntrinsicCalibration{
					OpticalCenterX: 180,
					OpticalCenterY: 320,
				},
				Extrinsics: &calibration.CoordinateTransformation{
					RodriguesRotationX: 1,
				},
			},
		},
	})

	SendProto(vehicleConn, msg0)
	<-doneChan
	SendProto(vehicleConn, makeGPS(1.1, 2.2))
	<-doneChan
	SendProto(vehicleConn, makeGPS(3.3, 4.4))
	<-doneChan

	var resp struct {
		Vehicle *teleop.VehicleView `json:"vehicle"`
	}
	req, err := makeReq(server.URL, fmt.Sprintf("/api/v1/vehicle/%s", sessionCookie.Value), "GET", nil, adminCookie)
	require.NoError(t, err)
	reply, err := client.Do(req)
	require.NoError(t, err)
	defer reply.Body.Close()
	blob, err := ioutil.ReadAll(reply.Body)
	require.NoError(t, err)
	err = json.Unmarshal(blob, &resp)
	require.NoError(t, err)

	require.NotNil(t, resp.Vehicle)
	require.NotNil(t, resp.Vehicle.Location)

	assert.Equal(t, 3.3, resp.Vehicle.Location.Location.Lat)
	assert.Equal(t, 4.4, resp.Vehicle.Location.Location.Lon)

	require.Len(t, resp.Vehicle.Cameras, 2)
}

func TestApp_GetFrames(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()
	framesURL := fmt.Sprintf("/api/v1/vehicle/%s/camera/front", vehicleID)
	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())

	vehicleConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID), sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	// failure case vehicle did not send manifest yet
	req, err := makeReq(server.URL, fmt.Sprintf("%s?latest=true", framesURL), "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.NotEqual(t, http.StatusOK, resp.Status)

	manifest := teleop.NewManifestMessage(&teleopproto.Manifest{
		Cameras: []*teleopproto.Camera{
			&teleopproto.Camera{
				Device:     teleop.NewDevice("front"),
				Role:       hal1.CameraId_FrontFisheye,
				Width:      640,
				Height:     360,
				Intrinsics: &calibration.CameraIntrinsicCalibration{},
				Extrinsics: &calibration.CoordinateTransformation{},
			},
			&teleopproto.Camera{
				Device:     teleop.NewDevice("back"),
				Role:       hal1.CameraId_RearFisheye,
				Width:      1080,
				Height:     720,
				Intrinsics: &calibration.CameraIntrinsicCalibration{},
				Extrinsics: &calibration.CoordinateTransformation{},
			},
		},
	})

	SendProto(vehicleConn, manifest)
	<-doneChan

	// failure case no frames sent yet
	req, err = makeReq(server.URL, fmt.Sprintf("%s?latest=true", framesURL), "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.NotEqual(t, http.StatusOK, resp.Status)

	SendProto(vehicleConn, makeFrame("frame1", "front"))
	<-doneChan

	SendProto(vehicleConn, makeFrame("frame2", "front"))
	<-doneChan

	SendProto(vehicleConn, makeFrame("frame3", "front"))
	<-doneChan

	/* Related to the ci timing issues
	time.Sleep(100 * time.Millisecond)

	thirdFrameTime := time.Now().UnixNano()

	time.Sleep(100 * time.Millisecond)
	*/

	SendProto(vehicleConn, makeFrame("frame2", "front"))
	<-doneChan

	SendProto(vehicleConn, makeFrame("frame3", "front"))
	<-doneChan

	// failure case not authed yet
	req, err = makeReq(server.URL, fmt.Sprintf("%s?latest=true", framesURL), "GET", nil, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.NotEqual(t, http.StatusOK, resp.Status)

	// requesting just the latest should succeed
	req, err = makeReq(server.URL, fmt.Sprintf("%s?latest=true", framesURL), "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	frame := new(data.CameraSample)
	err = json.NewDecoder(resp.Body).Decode(frame)
	require.NoError(t, err)

	// request frames since the third frame was written
	/*  These tests are based on server timings and we can't guarantee their result on ci

	req, err = makeReq(server.URL, framesURL, "GET", nil, adminCookie)
	require.NoError(t, err)

	q := req.URL.Query()
	q.Add("since", strconv.Itoa(int(thirdFrameTime)))
	req.URL.RawQuery = q.Encode()

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	frames := new(data.CameraSamples)
	err = json.NewDecoder(resp.Body).Decode(frames)
	require.NoError(t, err)
	assert.Len(t, *frames, 3)

	// request frames since the third frame was written with limit
	limit := 2
	req, err = makeReq(server.URL, framesURL, "GET", nil, adminCookie)
	require.NoError(t, err)

	ql := req.URL.Query()
	ql.Add("since", strconv.Itoa(int(thirdFrameTime)))
	ql.Add("limit", strconv.Itoa(limit))
	req.URL.RawQuery = ql.Encode()

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()
	frames = new(data.CameraSamples)
	err = json.NewDecoder(resp.Body).Decode(frames)
	require.NoError(t, err)
	assert.Len(t, *frames, limit)
	*/

	// request fewer than the max should succeed
	limit := 2
	req, err = makeReq(server.URL, fmt.Sprintf("%s?limit=%d", framesURL, limit), "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	frames := new(data.CameraSamples)
	err = json.NewDecoder(resp.Body).Decode(frames)
	require.NoError(t, err)
	assert.Len(t, *frames, limit)

	// request more than the max should succeed and return the max
	limit = 10
	req, err = makeReq(server.URL, fmt.Sprintf("%s?limit=%d", framesURL, limit), "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	newFrames := new(data.CameraSamples)
	err = json.NewDecoder(resp.Body).Decode(newFrames)
	require.NoError(t, err)
	assert.Len(t, *newFrames, 5)
}

func TestApp_DockVehicle(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()

	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())
	vehicleConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID), sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	operatorConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/subscribe", sessionCookie.Value), authHeader)
	require.NoError(t, err)
	defer operatorConn.Close()
	<-doneChan

	stations := []uint64{123, 456}
	observations := teleop.NewDockingObservation(time.Now(), stations)

	err = SendProto(vehicleConn, observations)
	require.NoError(t, err)
	<-doneChan
	<-doneChan

	recvMsg, err := RecvVehicleMsg(operatorConn)
	require.NoError(t, err)
	assert.Equal(t, stations, recvMsg.GetDockingObservation().GetStationIds())

	err = tSendJSONPB(operatorConn, teleop.NewDockCommand(stations[0]))
	require.NoError(t, err)
	<-doneChan

	var msg teleopproto.BackendMessage
	RecvProto(vehicleConn, &msg)
	assert.Equal(t, stations[0], msg.GetDockCommand().GetStationId())

	distanceX, distanceY, angle := float32(1.2), float32(1.2), float32(0.1)
	status := teleop.NewDockingStatus(teleopproto.DockingStatus_INPROGRESS, distanceX, distanceY, angle)
	err = SendProto(vehicleConn, status)
	require.NoError(t, err)
	<-doneChan
	<-doneChan

	recvMsg, err = RecvVehicleMsg(operatorConn)
	require.NoError(t, err)
	assert.Equal(t, distanceX, recvMsg.GetDockingStatus().GetRemainingDistanceX())
	assert.Equal(t, distanceY, recvMsg.GetDockingStatus().GetRemainingDistanceY())
	assert.Equal(t, angle, recvMsg.GetDockingStatus().GetRemainingAngle())
	assert.Equal(t, teleopproto.DockingStatus_INPROGRESS, recvMsg.GetDockingStatus().GetStatus())
}

func tSendJSON(conn *websocket.Conn, obj interface{}) error {
	buf, err := json.Marshal(obj)
	if err != nil {
		return err
	}

	return conn.WriteMessage(websocket.TextMessage, buf)
}

func tSendJSONPB(conn *websocket.Conn, pb proto.Message) error {
	buf, err := MarshalJSONPB(pb)
	if err != nil {
		return err
	}

	return conn.WriteMessage(websocket.TextMessage, buf)
}
