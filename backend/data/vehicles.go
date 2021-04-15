package data

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
	"time"

	"github.com/aws/aws-sdk-go/aws"
	"github.com/aws/aws-sdk-go/service/s3/s3manager"
	"github.com/garyburd/redigo/redis"
	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

const (
	vehicleKeyFmt        = "vehicle/%s"
	cameraInfoKeyFmt     = "camera/%s"
	vehicleSessionKeyFmt = "session/%s"
)

// TokenState is an enum for the state that a vehicle token is in and wether it should be allowed
type TokenState int

const (
	// Inactive is when a token was just generated and authorized user has not yet approved it
	Inactive TokenState = iota
	// Valid is that the token can be used to authenticate and identify a vehicle
	Valid
	// InValid is used for when we retired an old vehicle token
	InValid
)

// VehicleStatus in an enum representing the vehicle status
type VehicleStatus int

// The VehicleStatus enum values
const (
	StatusActive VehicleStatus = iota
	StatusInactive
	StatusOffline
	StatusNetworkDrop
)

// MarshalText implements encoding.TextMarshaler
func (x VehicleStatus) MarshalText() ([]byte, error) {
	switch x {
	case StatusActive:
		return []byte("active"), nil
	case StatusInactive:
		return []byte("inactive"), nil
	case StatusOffline:
		return []byte("offline"), nil
	case StatusNetworkDrop:
		return []byte("network_drop"), nil
	default:
		return nil, fmt.Errorf("invalid VehicleStatus: %d", x)
	}
}

// UnmarshalText implements encoding.TextUnmarshaler
func (x *VehicleStatus) UnmarshalText(b []byte) error {
	switch string(b) {
	case "active":
		*x = StatusActive
		return nil
	case "inactive":
		*x = StatusInactive
		return nil
	case "offline":
		*x = StatusOffline
		return nil
	case "network_drop":
		*x = StatusNetworkDrop
		return nil
	default:
		return fmt.Errorf("cannot parse %q as vehicle status", string(b))
	}
}

// Vehicle contains a vehicles ID and auth token
type Vehicle struct {
	// persistant values in postgres
	CameraIDs []string        `json:"-"`
	Tokens    []*VehicleToken `json:"-"`
	// cached values in redis
	VehicleSession
	// cached values that are loaded as needed
	Cameras map[string]*Camera `json:"-"`
}

// Location is the last saved location for the vehicle
type Location struct {
	Timestamp time.Time `json:"timestamp"`
	Lat       float64   `json:"lat"`
	Lon       float64   `json:"lon"`
	Alt       float64   `json:"alt"`
}

// Controller is a User that is viewing/controlling a vehicle
type Controller struct {
	UserID       string   `json:"user_id"`
	ConnectionID string   `json:"connection_id"`
	CameraID     []string `json:"cameras"`
}

// VehicleSession contains all of the session information for a vehicle in redis
type VehicleSession struct {
	VehicleID      string                    `json:"vehicle_id"`
	Status         VehicleStatus             `json:"status"`
	WebRTCSessions map[string]*WebRTCSession `json:"webrtc_session_states"`
	Controlling    *Controller               `json:"controller"`
	Viewers        []*Controller             `json:"viewers"`
	LastLocation   *Location                 `json:"last_location"`
}

// VehicleToken is a struct containing the token value and the state it's in
type VehicleToken struct {
	Token      string     `json:"-"`
	TokenState TokenState `json:"-"`
}

// WebRTCSession contains the vehicles current session states with a user
type WebRTCSession struct {
	SdpStatus teleopproto.SDPStatus
	RTCStatus teleopproto.RTCStatus
}

// FindViewer returns the viewer that has the username given
func (v *VehicleSession) FindViewer(userID string) (*Controller, error) {
	for _, operator := range v.Viewers {
		if operator.UserID == userID {
			return operator, nil
		}
	}

	return nil, fmt.Errorf("user with id: %s is not a viewer", userID)
}

// AuthVehicle is the PersistentStorage implementation to authenticate a vehicle with a given token returning the tokens state
func (p *PersistentStorage) AuthVehicle(vehicleID, token string) (TokenState, error) {
	defer func(now time.Time) {
		requestTime.With("method", authVehicle, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var stmt bytes.Buffer
	stmt.WriteString("SELECT token_state FROM vehicles_auth ")
	stmt.WriteString("WHERE vehicle_id=$1 AND token=$2 ")
	stmt.WriteString("ORDER BY created_at DESC LIMIT 1")

	row := p.db.QueryRow(stmt.String(), vehicleID, token)

	var state int
	err := row.Scan(&state)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", authVehicle, "data_store", postgres).Add(1)
		return InValid, err
	}

	return TokenState(state), nil
}

// CreateVehicle is the PersistentStorage implementation to create a vehicle record
func (p *PersistentStorage) CreateVehicle(vehicleID string) (*Vehicle, error) {
	defer func(now time.Time) {
		requestTime.With("method", createVehicle, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var stmt bytes.Buffer
	stmt.WriteString("INSERT INTO vehicles(id) ")
	stmt.WriteString("Values($1) RETURNING id")

	row := p.db.QueryRow(stmt.String(), vehicleID)

	vehicle := new(Vehicle)
	err := row.Scan(&vehicle.VehicleID)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", createVehicle, "data_store", postgres).Add(1)
		return nil, err
	}

	return vehicle, nil
}

// CreateVehicleAuth is the PersistentStorage implementation to update the token for a vehicle
func (p *PersistentStorage) CreateVehicleAuth(vehicleID, token string, tokenState TokenState) error {
	defer func(now time.Time) {
		requestTime.With("method", createVehicleAuth, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var stmt bytes.Buffer
	stmt.WriteString("INSERT INTO vehicles_auth(vehicle_id, token, token_state) ")
	stmt.WriteString("VALUES($1, $2, $3)")

	_, err := p.db.Exec(stmt.String(), vehicleID, token, int(tokenState))
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", createVehicleAuth, "data_store", postgres).Add(1)
		return err
	}

	return nil
}

// UpdateVehicleAuth is the PersistentStorage implementation to update the token for a vehicle
func (p *PersistentStorage) UpdateVehicleAuth(vehicleID, token string, tokenState TokenState) error {
	defer func(now time.Time) {
		requestTime.With("method", updateVehicleAuth, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var stmt bytes.Buffer
	stmt.WriteString("UPDATE vehicles_auth SET token=$2, token_state=$3")
	stmt.WriteString("WHERE vehicle_id=$1")

	_, err := p.db.Exec(stmt.String(), vehicleID, token, int(tokenState))
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", updateVehicleAuth, "data_store", postgres).Add(1)
		return err
	}

	return nil
}

// SetVehicleLocation is the PersistentStorage implementation to set the last known gps cords
func (p *PersistentStorage) SetVehicleLocation(vehicleID string, location *Location) error {
	defer func(now time.Time) {
		requestTime.With("method", setVehicleLocation, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var stmt bytes.Buffer
	stmt.WriteString("INSERT INTO vehicles_locations(vehicle_id, time_stamp, lat, lon, alt) ")
	stmt.WriteString("VALUES($1, $2, $3, $4, $5)")

	_, err := p.db.Exec(stmt.String(), vehicleID, location.Timestamp, location.Lat,
		location.Lon, location.Alt)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", setVehicleLocation, "data_store", postgres).Add(1)
		return err
	}

	return nil
}

// SaveCameraFrame is the PersistentStorage implementation turn the images contents into a url for storage
func (p *PersistentStorage) SaveCameraFrame(vehicleID string, frame *CameraSample) error {
	defer func(now time.Time) {
		requestTime.With("method", saveCameraFrame, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	if frame.CameraID == "" {
		operationsErrorTotal.With("error", ErrNoCamera.Error(), "method", saveCameraFrame, "data_store", postgres).Add(1)
		return ErrNoCamera
	}

	if frame.Timestamp.IsZero() {
		operationsErrorTotal.With("error", ErrBadTimeStamp.Error(), "method", saveCameraFrame, "data_store", postgres).Add(1)
		return ErrBadTimeStamp
	}

	fileKey := fmt.Sprintf("%s/%s-%s", vehicleID, frame.CameraID, frame.Timestamp.String())
	body := bytes.NewReader(frame.Content)

	result, err := p.s3.s3.Upload(&s3manager.UploadInput{
		Bucket: aws.String(p.s3.bucketName),
		Key:    aws.String(fileKey),
		Body:   body,
	})
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", saveCameraFrame, "data_store", postgres).Add(1)
		return err
	}
	frame.URL = result.Location

	return nil
}

// SetVehicleCameras is the PersistentStorage implementation to add a variadic number of camera ids
func (p *PersistentStorage) SetVehicleCameras(vehicleID string, cameraIDs ...string) error {
	defer func(now time.Time) {
		requestTime.With("method", setVehicleCameras, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var vals []interface{}
	vals = append(vals, vehicleID)
	var stmt bytes.Buffer
	stmt.WriteString("INSERT INTO vehicles_cameras(vehicle_id, id) VALUES ")
	for i, cameraID := range cameraIDs {
		stmtAddition := fmt.Sprintf("($1, $%d), ", i+2)
		// if we're at the end do not add the trailing comma
		if i+1 == len(cameraIDs) {
			stmtAddition = strings.TrimSuffix(stmtAddition, ", ")
		}

		stmt.WriteString(stmtAddition)
		vals = append(vals, cameraID)
	}

	_, err := p.db.Exec(stmt.String(), vals...)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", setVehicleCameras, "data_store", postgres).Add(1)
		return err
	}

	return nil
}

// FindVehicle is the PersistentStorage implementation to find a vehicle record by id
func (p *PersistentStorage) FindVehicle(vehicleID string) (*Vehicle, error) {
	defer func(now time.Time) {
		requestTime.With("method", findVehicle, "data_store", postgres).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var stmt bytes.Buffer
	stmt.WriteString("SELECT vehicle_id, token, token_state FROM vehicles_auth WHERE vehicle_id=$1 ")
	stmt.WriteString("ORDER BY created_at DESC LIMIT 1")

	row := p.db.QueryRow(stmt.String(), vehicleID)

	vehicle := new(Vehicle)
	vehicle.Tokens = append(vehicle.Tokens, &VehicleToken{})
	err := row.Scan(&vehicle.VehicleID, &vehicle.Tokens[0].Token, &vehicle.Tokens[0].TokenState)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", findVehicle, "data_store", postgres).Add(1)
		return nil, err
	}
	return vehicle, nil
}

// GetVehicleCache is the PersistentStorage implementation to get the vehicle values that are stored in redis
func (p *PersistentStorage) GetVehicleCache(vehicleSessionID string) (*VehicleSession, error) {
	defer func(now time.Time) {
		requestTime.With("method", getVehicleCache, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	key := fmt.Sprintf(vehicleSessionKeyFmt, vehicleSessionID)
	vehicle := new(VehicleSession)

	if err := p.getRedisJSON("GET", key, "", vehicle); err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", getVehicleCache, "data_store", redisStore).Add(1)
		return nil, err
	}

	return vehicle, nil
}

// SetVehicleCache is the PersistentStorage implementation to set the vehicle values that are stored in redis
func (p *PersistentStorage) SetVehicleCache(sessionID string, vehicle *VehicleSession) error {
	defer func(now time.Time) {
		requestTime.With("method", setVehicleCache, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	key := fmt.Sprintf(vehicleSessionKeyFmt, sessionID)

	err := p.setRedisJSON("SET", key, "", vehicle)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", setVehicleCache, "data_store", redisStore).Add(1)
		return err
	}

	return nil
}

// GetVehicleCameraCache is the PersistentStorage implementation to get a vehicles camera data from redis
func (p *PersistentStorage) GetVehicleCameraCache(vehicleID, cameraID string) (*Camera, error) {
	defer func(now time.Time) {
		requestTime.With("method", getVehicleCameraCache, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	key := fmt.Sprintf(vehicleKeyFmt, vehicleID)
	cameraKey := fmt.Sprintf(cameraInfoKeyFmt, cameraID)
	camera := new(Camera)

	if err := p.getRedisJSON("HGET", key, cameraKey, camera); err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", getVehicleCameraCache, "data_store", redisStore).Add(1)
		return nil, err
	}

	return camera, nil
}

// GetVehicleCamerasCache is the PersistentStorage implementation to get all of a vehicles camera data from redis
func (p *PersistentStorage) GetVehicleCamerasCache(vehicleID string) (map[string]*Camera, error) {
	defer func(now time.Time) {
		requestTime.With("method", getVehicleCamerasCache, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	conn := p.pool.Get()
	defer conn.Close()

	key := fmt.Sprintf(vehicleKeyFmt, vehicleID)
	keys, err := redis.Strings(conn.Do("HKEYS", key))
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", getVehicleCamerasCache, "data_store", redisStore).Add(1)
		return nil, err
	}

	result := make(map[string]*Camera)
	for _, keyName := range keys {
		if !strings.HasPrefix(keyName, "camera/") {
			continue
		}

		camera := new(Camera)
		err = p.getRedisJSON("HGET", key, keyName, camera)
		if err != nil {
			operationsErrorTotal.With("error", err.Error(), "method", getVehicleCamerasCache, "data_store", redisStore).Add(1)
			return nil, err
		}

		cameraID := strings.TrimPrefix(keyName, "camera/")
		result[cameraID] = camera
	}

	return result, nil
}

// SetVehicleCameraCache is the PersistentStorage implementation to set a vehicle's camera data in redis
func (p *PersistentStorage) SetVehicleCameraCache(vehicleID string, camera *Camera) error {
	defer func(now time.Time) {
		requestTime.With("method", setVehicleCameraCache, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	key := fmt.Sprintf(vehicleKeyFmt, vehicleID)
	cameraKey := fmt.Sprintf(cameraInfoKeyFmt, camera.ID)

	if err := p.setRedisJSON("HSET", key, cameraKey, camera); err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", setVehicleCameraCache, "data_store", redisStore).Add(1)
		return err
	}

	return nil
}

func (p *PersistentStorage) getRedisJSON(cmd, key, segmentKey string, value interface{}) error {
	conn := p.pool.Get()
	defer conn.Close()

	var jBlob []byte
	var err error

	if cmd == "HGET" {
		jBlob, err = redis.Bytes(conn.Do(cmd, key, segmentKey))
	} else if cmd == "GET" {
		jBlob, err = redis.Bytes(conn.Do(cmd, key))
	} else {
		return fmt.Errorf("%s is not a suported command currently", cmd)
	}

	if err != nil {
		return err
	}

	return json.Unmarshal(jBlob, &value)
}

func (p *PersistentStorage) setRedisJSON(cmd, key, segmentKey string, value interface{}) error {
	conn := p.pool.Get()
	defer conn.Close()

	jBlob, err := json.Marshal(value)
	if err != nil {
		return err
	}

	if cmd == "HSET" {
		_, err = conn.Do(cmd, key, segmentKey, jBlob)
	} else if cmd == "SET" {
		_, err = conn.Do(cmd, key, jBlob)
	} else {
		return fmt.Errorf("%s is not a suported command currently", cmd)
	}

	if err != nil {
		return err
	}

	return nil
}

// AuthVehicle is the localStorage implementation to authenticate a vehicle with a given token returning the tokens state
func (l *localStorage) AuthVehicle(vehicleID, token string) (TokenState, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	vehicle, ok := l.vehicles[vehicleID]
	if !ok {
		return InValid, ErrNoVehicle
	}

	for _, state := range vehicle.Tokens {
		if state.Token == token {
			return state.TokenState, nil
		}
	}

	return InValid, fmt.Errorf("unable to find vehicle auth with id %s", vehicleID)

}

// CreateVehicle is the localStorage implementation to create a vehicle record
func (l *localStorage) CreateVehicle(vehicleID string) (*Vehicle, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	vehicle, ok := l.vehicles[vehicleID]
	if ok {
		return vehicle, nil
	}

	newVehicle := &Vehicle{
		VehicleSession: VehicleSession{
			VehicleID: vehicleID,
		},
	}

	l.vehicles[vehicleID] = newVehicle

	return newVehicle, nil
}

// CreateVehicleAuth is the localStorage implementation to update the token for a vehicle
func (l *localStorage) CreateVehicleAuth(vehicleID, token string, tokenState TokenState) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	vehicle, ok := l.vehicles[vehicleID]
	if !ok {
		return ErrNoVehicle
	}

	state := &VehicleToken{
		Token:      token,
		TokenState: tokenState,
	}

	vehicle.Tokens = append(vehicle.Tokens, state)

	l.vehicles[vehicleID] = vehicle

	return nil
}

// UpdateVehicleAuth is the localStorage implementation to update the token for a vehicle
func (l *localStorage) UpdateVehicleAuth(vehicleID, token string, tokenState TokenState) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	vehicle, ok := l.vehicles[vehicleID]
	if !ok {
		return ErrNoVehicle
	}

	for _, state := range vehicle.Tokens {
		if state.Token == token {
			state.TokenState = tokenState
		}
	}

	l.vehicles[vehicleID] = vehicle

	return nil
}

// SaveCameraFrame is the localStorage implementation turn the images contents into a url for storage
func (l *localStorage) SaveCameraFrame(vehicleID string, frame *CameraSample) error {
	if frame.CameraID == "" {
		return ErrNoCamera
	}

	if frame.Timestamp.IsZero() {
		return ErrBadTimeStamp
	}

	imagePath := fmt.Sprintf("%s/%s", l.imageDir, vehicleID)
	fileName := fmt.Sprintf("%s/%s-%s", imagePath, frame.CameraID, frame.Timestamp.String())

	// ensure that the dir we want to write to exists
	if _, err := os.Stat(imagePath); os.IsNotExist(err) {
		if err = os.MkdirAll(imagePath, os.ModePerm); err != nil {
			return err
		}
	}

	err := ioutil.WriteFile(fileName, frame.Content, os.ModePerm)
	if err != nil {
		return err
	}

	// set the frame url to the pointer sample
	frame.URL = fmt.Sprintf("http://localhost:8000/images/%s/%s-%s", vehicleID, frame.CameraID, frame.Timestamp.String())

	return nil
}

// SetVehicleCameras is the localStorage implementation to add a variadic number of camera ids
func (l *localStorage) SetVehicleCameras(vehicleID string, cameraIDs ...string) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	vehicle, ok := l.vehicles[vehicleID]
	if !ok {
		vehicle = &Vehicle{
			VehicleSession: VehicleSession{
				VehicleID: vehicleID,
			},
		}

		l.vehicles[vehicleID] = vehicle
	}
	vehicle.CameraIDs = append(vehicle.CameraIDs, cameraIDs...)

	if vehicle.Cameras == nil {
		vehicle.Cameras = make(map[string]*Camera)
	}

	return nil
}

// FindVehicle is the localStorage implementation to find a vehicle record by id
func (l *localStorage) FindVehicle(vehicleID string) (*Vehicle, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	vehicle, ok := l.vehicles[vehicleID]
	if !ok {
		return nil, ErrNoVehicle
	}

	return vehicle, nil
}

// SetVehicleLocation is the localStorage implementation to set the last known gps cords
func (l *localStorage) SetVehicleLocation(vehicleID string, location *Location) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	vehicle, ok := l.vehicles[vehicleID]
	if !ok {
		return ErrNoVehicle
	}

	vehicle.LastLocation = location
	return nil
}

// GetVehicleCache is the localStorage implementation to get the vehicle values that are stored in redis
func (l *localStorage) GetVehicleCache(sessionID string) (*VehicleSession, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	vehicle, ok := l.vehicleSessions[sessionID]
	if !ok {
		return nil, ErrNoVehicle
	}

	return vehicle, nil
}

// GetVehicleCache is the localStorage implementation to set the vehicle values that are stored in redis
func (l *localStorage) SetVehicleCache(sessionID string, vehicleSession *VehicleSession) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	vehicle, ok := l.vehicles[vehicleSession.VehicleID]
	if !ok {
		vehicle = &Vehicle{
			VehicleSession: VehicleSession{
				VehicleID: vehicleSession.VehicleID,
				Status:    StatusActive,
			},
		}
	}
	l.vehicles[vehicleSession.VehicleID] = vehicle

	l.vehicleSessions[sessionID] = vehicleSession

	return nil
}

// GetVehicleCameraCache is the localStorage implementation to get a vehicles camera data from redis
func (l *localStorage) GetVehicleCameraCache(vehicleID, cameraID string) (*Camera, error) {
	vehicle, err := l.FindVehicle(vehicleID)
	if err != nil {
		return nil, err
	}

	camera, ok := vehicle.Cameras[cameraID]
	if !ok {
		return nil, ErrNoCamera
	}

	return camera, nil
}

// GetVehicleCamerasCache is the localStorage implementation to get all of a vehicles camera data from redis
func (l *localStorage) GetVehicleCamerasCache(vehicleID string) (map[string]*Camera, error) {
	vehicle, err := l.FindVehicle(vehicleID)
	if err != nil {
		return nil, err
	}

	return vehicle.Cameras, nil
}

// SetVehicleCameraCache is the localStorage implementation to set a vehicle's camera data in redis
func (l *localStorage) SetVehicleCameraCache(vehicleID string, camera *Camera) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	vehicle, ok := l.vehicles[vehicleID]
	if !ok {
		return ErrNoVehicle
	}

	if vehicle.Cameras == nil {
		vehicle.Cameras = make(map[string]*Camera)
	}

	vehicle.Cameras[camera.ID] = camera
	return nil
}

// NewVehicle creates a new data vehicle object from an id
func NewVehicle(id string) *Vehicle {
	return &Vehicle{
		VehicleSession: VehicleSession{
			VehicleID: id,
		},
	}
}
