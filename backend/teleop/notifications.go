package teleop

import (
	"encoding/json"
	"fmt"
	"strings"
)

// NotificationType is used for all of the different types of notifications for users
type NotificationType uint32

// UnmarshalJSON is the json unmarshaler implementation for NotificationType
func (n *NotificationType) UnmarshalJSON(b []byte) error {
	var s string
	if err := json.Unmarshal(b, &s); err != nil {
		return err
	}

	switch strings.ToLower(s) {
	case "vehicle_disconnected":
		*n = VehicleDisconnected
	default:
		return fmt.Errorf("unable to parse %s as a NotificationType", s)
	}

	return nil
}

// MarshalJSON is the json marshaler implementation for NotificationType
func (n NotificationType) MarshalJSON() ([]byte, error) {
	var s string
	switch n {
	case VehicleDisconnected:
		s = "vehicle_disconnected"
	default:
		return nil, fmt.Errorf("unable to parse %d as a string", int(n))
	}

	return json.Marshal(s)
}

const (
	// VehicleDisconnected vehicle was unexpectedly disconnected
	VehicleDisconnected NotificationType = iota
)

// Notification is a generic notification message to be sent to users listening
type Notification struct {
	Type    NotificationType `json:"type"`
	Message string           `json:"message"`
	Payload interface{}      `json:"payload"`
}

// NewPGDeadNotification is a notification to alert the user that point and go died
func NewPGDeadNotification(sessionID string) *Notification {
	return &Notification{
		Type:    VehicleDisconnected,
		Payload: sessionID,
		Message: "Point and go on the vehicle has been disconnected",
	}
}
