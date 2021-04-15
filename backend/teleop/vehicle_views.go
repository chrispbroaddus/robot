package teleop

import (
	"encoding/json"
	"fmt"
	"strings"
	"time"

	"github.com/zippyai/zippy/backend/data"
)

// VehicleAuthView is the client view for vehicle authentication information
type VehicleAuthView struct {
	VehicleID string `json:"vehicleID,omitempty"`
	Token     string `json:"token"`
}

// VehicleView contains status information for a connected vehicle
type VehicleView struct {
	ID        string             `json:"id"`
	VehicleID string             `json:"vehicle_id"`
	Name      string             `json:"name,omitempty"`
	Status    data.VehicleStatus `json:"status"`
	Location  *LocationSample    `json:"location"`
	Operator  *Operator          `json:"operator,omitempty"`
	Viewers   []*Operator        `json:"viewers,omitempty"`
	Cameras   []Camera           `json:"cameras,omitempty"`
}

// VehiclesView is a wrapper struct for multiple vehicle views
type VehiclesView struct {
	Vehicles []*VehicleView `json:"vehicles"`
}

// NewVehicleView returns a vehicle view for the given data Vehicle
func NewVehicleView(sessionID string, vehicle *data.VehicleSession, cameras map[string]*data.Camera) *VehicleView {
	operator := new(Operator)
	if vehicle.Controlling != nil {
		operator.Name = vehicle.Controlling.UserID
	}

	viewers := make([]*Operator, len(vehicle.Viewers))
	for i, viewer := range vehicle.Viewers {
		viewers[i] = &Operator{
			Name: viewer.UserID,
		}
	}

	lastLocation := new(LocationSample)
	if vehicle.LastLocation != nil {
		lastLocation.Timestamp = vehicle.LastLocation.Timestamp
		lastLocation.Location = Location{
			Lat: vehicle.LastLocation.Lat,
			Lon: vehicle.LastLocation.Lon,
			Alt: vehicle.LastLocation.Alt,
		}
	}

	resultCameras := make([]Camera, len(cameras))
	i := 0
	for key, cacheCamera := range cameras {
		resultCameras[i] = Camera{
			ID:         key,
			Role:       cacheCamera.Role,
			Width:      cacheCamera.Width,
			Height:     cacheCamera.Height,
			Intrinsics: cacheCamera.Intrinsics,
			Extrinsics: cacheCamera.Extrinsics,
		}
		i++
	}

	return &VehicleView{
		ID:        sessionID,
		VehicleID: vehicle.VehicleID,
		Location:  lastLocation,
		Status:    vehicle.Status,
		Operator:  operator,
		Viewers:   viewers,
		Cameras:   resultCameras,
	}
}

// SimulatorStatus is an enum for the different sim notifier status can be
type SimulatorStatus int

const (
	// Ready is for when a sim has been fully booted up
	Ready SimulatorStatus = iota
	// Error is for when ever an error is returned during the setup
	Error
	// Ping is used to keep the connection alive and the client still listening
	Ping
	// Start is used to let the client know they're now listening
	Start
)

// SimulatorEvent message sent when a simulator is started successfully
type SimulatorEvent struct {
	ID        string          `json:"id"`
	Status    SimulatorStatus `json:"status"`
	TimeStamp time.Time       `json:"timestamp"`
	Message   string          `json:"message"`
}

// MarshalJSON is the json marshaler implementation for SimulatorStatus
func (s SimulatorStatus) MarshalJSON() ([]byte, error) {
	var status string
	switch s {
	case Ready:
		status = "ready"
	case Error:
		status = "error"
	case Ping:
		status = "ping"
	case Start:
		status = "start"
	default:
		return nil, fmt.Errorf("unable to parse %d as a string", int(s))
	}

	return json.Marshal(status)
}

// UnmarshalJSON is the json unmarshaler implementation for SimulatorStatus
func (s *SimulatorStatus) UnmarshalJSON(b []byte) error {
	var status string
	if err := json.Unmarshal(b, &status); err != nil {
		return err
	}

	switch strings.ToLower(status) {
	case "ready":
		*s = Ready
	case "error":
		*s = Error
	case "ping":
		*s = Ping
	case "start":
		*s = Start
	default:
		return fmt.Errorf("unable to parse %s as SimulatorStatus", status)
	}

	return nil
}
