package services

import (
	"errors"
	"fmt"
	"net/http"
	"sort"
	"time"

	"github.com/garyburd/redigo/redis"
	"github.com/satori/go.uuid"
	"github.com/zippyai/zippy/backend/data"
	"github.com/zippyai/zippy/backend/teleop"
	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

var (
	// ErrVehicleAlreadyExists is returned if we try and create a user that exists
	ErrVehicleAlreadyExists = errors.New("vehicles with that id already exists")
	// ErrUserNotInControl is returned if a user is not the one controlling a vehicle
	ErrUserNotInControl = errors.New("user is not authorized to controll vehicle")
	// ErrNoFrames is returned if there are no camera frames saved yet for this camera
	ErrNoFrames = errors.New("there are no camera frames saved for this camera yet")
	// ErrTokenInactive is returned if the auth token used for a vehicle is not active yet
	ErrTokenInactive = errors.New("the token used for the vehicle has not been activated yet")
	// ErrTokenInvalid is returned if the auth token userd for the vehicle has since been invalidated
	ErrTokenInvalid = errors.New("the token used for the vehicle has been invalidated")
	// ErrWrongVehicleStatus is returned if the vehicle view that's gotten is not in the status requested
	ErrWrongVehicleStatus = errors.New("vehicle is not of the status that we wanted")
)

const (
	vehicleSessionExpiration = time.Hour * 5 // 5 hours
	// VehicleSessionCookieName name of the vehicle session cookie
	VehicleSessionCookieName = "vehicle_session"
)

// CreateVehicle ensures that a vehicle does not already exist then creates and saves a new vehicle
func CreateVehicle(store data.VehicleStorage, vehicleID string) (*data.Vehicle, error) {
	if vehicleID == "" {
		vehicleID = uuid.NewV4().String()
	}

	return store.CreateVehicle(vehicleID)
}

// CreateVehicleAuth adds the newly generated token to the vehicle storage object
func CreateVehicleAuth(store data.VehicleStorage, vehicleID, token string, tokenState data.TokenState) error {
	return store.CreateVehicleAuth(vehicleID, token, tokenState)
}

// UpdateVehicleAuth adds the newly generated token to the vehicle storage object
func UpdateVehicleAuth(store data.VehicleStorage, vehicleID, token string, tokenState data.TokenState) error {
	return store.UpdateVehicleAuth(vehicleID, token, tokenState)
}

// UserInControl gets the userId of the user that is currently in control of the vehicle
func UserInControl(store data.VehicleCache, vehicleID string) (string, error) {
	vehicle, err := store.GetVehicleCache(vehicleID)
	if err != nil {
		return "", err
	}

	if vehicle.Controlling == nil {
		return "", ErrUserNotInControl
	}

	return vehicle.Controlling.UserID, nil
}

// UserIsInControl returns an error if the user is not authorized and nil if they are
func UserIsInControl(store data.VehicleCache, vehicleID, userID string) error {
	controllerID, err := UserInControl(store, vehicleID)
	if err != nil {
		return err
	}

	if controllerID != userID {
		return ErrUserNotInControl
	}

	return nil
}

// AddVehicleViewer updates the vehicle object in storage to add new view relationship
func AddVehicleViewer(store data.VehicleStorage, sessionID, userID string) error {
	vehicle, err := store.GetVehicleCache(sessionID)
	if err != nil {
		return err
	}

	// don't do anything if that user is already a viewer
	if _, err := vehicle.FindViewer(userID); err == nil {
		return nil
	}

	vehicle.Viewers = append(vehicle.Viewers, &data.Controller{
		UserID: userID,
	})

	return store.SetVehicleCache(sessionID, vehicle)
}

// PromoteToController sets the view with name username to the controlling operator
func PromoteToController(store data.Storage, sessionID, userID string) error {
	vehicle, err := store.GetVehicleCache(sessionID)
	if err != nil {
		return err
	}

	// if the user is not currently viewing we need to create a new operator
	newController, err := vehicle.FindViewer(userID)
	if err != nil {
		newController = &data.Controller{
			UserID: userID,
		}
	}

	vehicle.Controlling = newController
	return store.SetVehicleCache(sessionID, vehicle)
}

// RemoveController sets the controller of the vehicle to nil
func RemoveController(store data.Storage, sessionID string) error {
	vehicle, err := store.GetVehicleCache(sessionID)
	if err != nil {
		return err
	}

	vehicle.Controlling = nil
	return store.SetVehicleCache(sessionID, vehicle)
}

// RegisterVehicle adds a record in redis for a newly connected vehicle
func RegisterVehicle(store data.VehicleCache, sessionID, vehicleID string) error {
	newVehicleSession := &data.VehicleSession{
		Status:    data.StatusActive,
		VehicleID: vehicleID,
	}

	err := store.SetVehicleCache(sessionID, newVehicleSession)
	if err != nil {
		return err
	}

	return nil
}

// GetLastFrame returns the latest frame for a vehicle's camera
func GetLastFrame(store data.VehicleCache, vehicleID, cameraID string) (*data.CameraSample, error) {
	camera, err := store.GetVehicleCameraCache(vehicleID, cameraID)
	if err != nil {
		return nil, err
	}

	if len(camera.Frames) <= 0 {
		return nil, ErrNoFrames
	}

	// sorts frames by timestamps latest
	sort.Sort(camera.Frames)

	return camera.Frames[0], nil
}

// UpdateLocation updated the location sent from the vehicle into storage
func UpdateLocation(store data.VehicleStorage, sessionID string, loc *teleop.LocationSample) error {
	location := locationToStorage(loc)

	vehicle, err := store.GetVehicleCache(sessionID)
	if err != nil {
		return err
	}
	vehicle.LastLocation = location

	// handle writing last location to storage when ever since this is not vital to be in sync
	go store.SetVehicleLocation(vehicle.VehicleID, location)

	return store.SetVehicleCache(sessionID, vehicle)
}

func locationToStorage(loc *teleop.LocationSample) *data.Location {
	return &data.Location{
		Timestamp: loc.Timestamp,
		Lat:       loc.Location.Lat,
		Lon:       loc.Location.Lon,
		Alt:       loc.Location.Alt,
	}
}

// SaveNewCameraFrame saves a camera frames contents to storage
func SaveNewCameraFrame(store data.VehicleStorage, vehicleID string, sample *teleop.CameraSample) error {
	newSample := frameToStorage(sample)
	err := store.SaveCameraFrame(vehicleID, newSample)
	if err != nil {
		return err
	}
	// gc the contents now that we've saved the content elsewhere
	newSample.Content = nil

	camera, err := store.GetVehicleCameraCache(vehicleID, sample.Camera)
	if err != nil {
		return err
	}

	newSample.Content = nil
	camera.Frames = append(camera.Frames, newSample)
	sort.Sort(camera.Frames)

	return store.SetVehicleCameraCache(vehicleID, camera)
}

func frameToStorage(sample *teleop.CameraSample) *data.CameraSample {
	return &data.CameraSample{
		CameraID:  sample.Camera,
		Timestamp: sample.Timestamp,
		Width:     sample.Image.Width,
		Height:    sample.Image.Height,
		Content:   sample.Image.Content,
	}
}

// UpdateSDPStatus updates the state that user is in with sdp handshaking with vehicle
func UpdateSDPStatus(store data.VehicleCache, sessionID, userID string, state teleopproto.SDPStatus) error {
	vehicle, err := store.GetVehicleCache(sessionID)
	if err != nil {
		return err
	}

	// ensure that the map is not nil
	if vehicle.WebRTCSessions == nil {
		vehicle.WebRTCSessions = make(map[string]*data.WebRTCSession)
	}

	// ensure that the pointer to the session state is not nil
	session, ok := vehicle.WebRTCSessions[userID]
	if !ok {
		session = &data.WebRTCSession{
			SdpStatus: state,
		}
	}

	vehicle.WebRTCSessions[userID] = session

	return store.SetVehicleCache(sessionID, vehicle)
}

// UpdateWebrtcStatus updates the state that user is in with  the vehicle for RTC connection
func UpdateWebrtcStatus(store data.VehicleCache, sessionID, userID string, state teleopproto.RTCStatus) error {
	vehicle, err := store.GetVehicleCache(sessionID)
	if err != nil {
		return err
	}

	// ensure that the map is not nil
	if vehicle.WebRTCSessions == nil {
		vehicle.WebRTCSessions = make(map[string]*data.WebRTCSession)
	}

	// ensure that the pointer to the session state is not nil
	session, ok := vehicle.WebRTCSessions[userID]
	if !ok {
		session = &data.WebRTCSession{
			RTCStatus: state,
		}
	}

	vehicle.WebRTCSessions[userID] = session

	return store.SetVehicleCache(sessionID, vehicle)
}

// SaveNewCameras sets the manifests cameras in storage to initialize them
func SaveNewCameras(store data.VehicleStorage, vehicleID string, protoCameras ...*teleopproto.Camera) (int, error) {
	cameras := camerasToStorage(protoCameras...)
	ids := make([]string, len(cameras))

	i := 0
	for id, cam := range cameras {
		ids[i] = id
		i++

		err := store.SetVehicleCameraCache(vehicleID, cam)
		if err != nil {
			return 0, err
		}
	}

	if err := store.SetVehicleCameras(vehicleID, ids...); err != nil {
		return 0, err
	}

	return len(cameras), nil
}

func camerasToStorage(cameras ...*teleopproto.Camera) map[string]*data.Camera {
	resultCameras := make(map[string]*data.Camera)
	for _, cam := range cameras {
		// if the device is nil we don't have enough information to save and should drop the camera
		if cam.GetDevice() == nil {
			continue
		}

		var extrinsics *data.Extrinsics
		var intrinsics *data.Intrinsics

		if cam.Extrinsics != nil {
			extrinsics = data.ExtrinsicsToStorage(cam.Extrinsics)
		}

		if cam.Intrinsics != nil {
			intrinsics = data.IntrinsicsToStorage(cam.Intrinsics)
		}

		cameraID := cam.GetDevice().Name
		resultCameras[cameraID] = &data.Camera{
			ID:         cameraID,
			Role:       cam.GetRole().String(),
			Width:      int(cam.Width),
			Height:     int(cam.Height),
			Serial:     cam.Device.SerialNumber,
			Extrinsics: extrinsics,
			Intrinsics: intrinsics,
		}
	}

	return resultCameras
}

// GetVehicleView returns the view for a single vehicle
func GetVehicleView(store data.Storage, sessionID, requesterID string, wantedStatus *data.VehicleStatus) (*teleop.VehicleView, error) {
	vehicle, err := store.GetVehicleCache(sessionID)
	if err != nil {
		if err == redis.ErrNil {
			return nil, data.ErrNoVehicle
		}

		return nil, err
	}

	// if we're looking for certain status we should drop all others
	if wantedStatus != nil && vehicle.Status != *wantedStatus {
		return nil, ErrWrongVehicleStatus
	}

	vehicleCameras, err := store.GetVehicleCamerasCache(vehicle.VehicleID)
	if err != nil {
		return nil, err
	}

	return teleop.NewVehicleView(sessionID, vehicle, vehicleCameras), nil
}

// ValidVehicleAuth auths a vehicle with a token. If the token is valid no error is returned otherwise an error is returned
func ValidVehicleAuth(store data.VehicleStorage, auth *teleop.VehicleAuthView) error {
	tokenState, err := store.AuthVehicle(auth.VehicleID, auth.Token)
	if err != nil {
		return err
	}

	switch tokenState {
	case data.Valid:
		return nil
	case data.Inactive:
		return ErrTokenInactive
	case data.InValid:
		return ErrTokenInvalid
	}

	return errors.New("token state is not valid")
}

// CreateNewVehicleSession registers a new session for the vehicle and returns a cookie containing the value
func CreateNewVehicleSession(store data.VehicleCache, vehicleID string) (*http.Cookie, error) {
	sessionID := uuid.NewV4().String()

	expireCookitTime := time.Now().Add(vehicleSessionExpiration)
	return &http.Cookie{
		Name:    VehicleSessionCookieName,
		Value:   sessionID,
		Expires: expireCookitTime,
		Path:    "/",
	}, nil
}

// UpdateSessionStatus updates a vehicles session state in redis
func UpdateSessionStatus(store data.VehicleCache, sessionID string, status data.VehicleStatus) error {
	session, err := store.GetVehicleCache(sessionID)
	if err != nil {
		return err
	}
	session.Status = status

	return store.SetVehicleCache(sessionID, session)
}

// CameraFramesSince returns camera frames since a timestamp of limit if it's present
func CameraFramesSince(store data.VehicleCache, vehicleID, cameraID string, since time.Time, limit int) (*data.CameraSamples, error) {
	camera, err := store.GetVehicleCameraCache(vehicleID, cameraID)
	if err != nil {
		return nil, err
	}

	samples := camera.Frames.SamplesSince(since)

	// if a limit is provided only return values up to the limit
	if limit > 0 {
		if limit < len(samples) {
			samples = samples[:limit]
		}
	}

	return &samples, nil
}

// LimitedCameraFrames returns the last n frames from a camera as defined by the limit
func LimitedCameraFrames(store data.VehicleCache, vehicleID, cameraID string, limit int) (*data.CameraSamples, error) {
	camera, err := store.GetVehicleCameraCache(vehicleID, cameraID)
	if err != nil {
		return nil, err
	}

	// ensure sorted
	sort.Sort(camera.Frames)

	samples := camera.Frames
	if len(samples) <= 0 {
		return nil, fmt.Errorf("there are no frames for the camerID: %s on vehicle: %s", cameraID, vehicleID)
	}

	// only take a limit subset if we have more frames than the limit
	if len(samples) > limit {
		samples = samples[:limit]
	}

	return &samples, nil
}

// VehicleActive returns a bool for if a vehicle session exists and is in an active state
func VehicleActive(store data.VehicleCache, sessionID string) bool {
	vehicleSession, err := store.GetVehicleCache(sessionID)
	if err != nil {
		return false
	}

	return vehicleSession.Status == data.StatusActive
}
