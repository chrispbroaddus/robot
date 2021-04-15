package teleop

import (
	"time"

	"github.com/zippyai/zippy/backend/data"

	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

// Location represents a latitude/longitude/altitude tuple
type Location struct {
	Lat float64 `json:"lat"`
	Lon float64 `json:"lon"`
	Alt float64 `json:"alt"`
}

// LocationSample is a timestamped location
type LocationSample struct {
	Timestamp time.Time `json:"timestamp"`
	Location  Location  `json:"location"`
}

// Camera contains information about a camera on a vehicle
type Camera struct {
	ID string `json:"id"` // e.g. "front", "rear"
	// Roles are defined in camera_id.pb.go when the enum is located
	Role       string           `json:"role"`
	Width      int              `json:"width"`  // width of images from this camera, in pixels
	Height     int              `json:"height"` // height of images from this camera, in pixels
	Intrinsics *data.Intrinsics `json:"intrinsics"`
	Extrinsics *data.Extrinsics `json:"extrinsics"`
}

// Operator contains information about a teleoperator
type Operator struct {
	Name string `json:"name"`
}

// Image represents a compressed camera frame
type Image struct {
	Width   int    `json:"width"`
	Height  int    `json:"height"`
	Content []byte `json:"content"`
}

// CameraSample is an image with a timestamp
type CameraSample struct {
	Timestamp time.Time `json:"timestamp"`
	Camera    string    `json:"camera"`
	Image     Image     `json:"image"`
}

// SDPMessage is the structure that is unmarshalled in the websocket connection for SDP connection
type SDPMessage struct {
	VideoRequest *VideoRequest    `json:"videoRequest,omitempty"`
	Request      *SDPRequest      `json:"sdpRequest,omitempty"`
	Confirmation *SDPConfirmation `json:"sdpConfirmation,omitempty"`
	ICE          *ICECandidate    `json:"iceCandidate,omitempty"`
}

// VideoRequest asks the vehicle to send an SDP offer for a specific camera
type VideoRequest struct {
	ConnectionID string `json:"connectionID"`
	CameraID     string `json:"cameraID"`
	Width        int    `json:"width"`
	Height       int    `json:"height"`
}

// SDPRequest is the information need to singal webrtc connections
type SDPRequest struct {
	ConnectionID string                `json:"connectionID"`
	Status       teleopproto.SDPStatus `json:"status"`
	SDP          string                `json:"sdp"`
}

// SDPConfirmation is sent to let the server and other peer know the connection status
type SDPConfirmation struct {
	ConnectionID string `json:"connectionID"`
	Connected    bool   `json:"connected"`
}

// ICECandidate used for sending all aditional candidates after initial request is made
type ICECandidate struct {
	ConnectionID  string `json:"connectionID"`
	Candidate     string `json:"candidate"`
	SDPMid        string `json:"sdpMid"`
	SDPMLineIndex int    `json:"sdpMlineIndex"`
}

// StartSimulator is the input used for starting up a new simulator on mission-control
type StartSimulator struct {
	VehicleName   string `json:"name"`
	SimulatorType string `json:"type"`
}

// NewSimulator is used to add or update available simulators to start in storage
type NewSimulator struct {
	Name           string `json:"name"`
	ConfigFilePath string `json:"config_file"`
}
