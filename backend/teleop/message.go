package teleop

import (
	"fmt"
	"time"

	"github.com/zippyai/zippy/backend/data"
	coreproto "github.com/zippyai/zippy/packages/core/proto"
	halproto "github.com/zippyai/zippy/packages/hal/proto"
	perceptionproto "github.com/zippyai/zippy/packages/perception/proto"
	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

// NewSystemTimestamp constructs a timestamp proto from a time.Time.
func NewSystemTimestamp(t time.Time) *coreproto.SystemTimestamp {
	nsec := t.Sub(gpsEpoch).Nanoseconds()
	if nsec < 0 {
		panic(fmt.Sprintf("cannot represent %v as SystemTimestamp (nsec=%d)", t, nsec))
	}
	return &coreproto.SystemTimestamp{
		Nanos: uint64(nsec),
	}
}

// NewJoystickMessage constructs a backend message containing a joystick command.
func NewJoystickMessage(s, w float64) *teleopproto.BackendMessage {
	return &teleopproto.BackendMessage{
		Payload: &teleopproto.BackendMessage_Joystick{
			Joystick: &teleopproto.JoystickCommand{
				LinearVelocity: s,
				Curvature:      w,
			},
		},
	}
}

// NewPointAndGoMessage constructs a backend message containing a joystick command.
func NewPointAndGoMessage(x, y float64, image, operator *coreproto.SystemTimestamp) *teleopproto.BackendMessage {
	return &teleopproto.BackendMessage{
		Payload: &teleopproto.BackendMessage_PointAndGo{
			PointAndGo: &teleopproto.PointAndGoCommand{
				ImageX:            x,
				ImageY:            y,
				ImageTimestamp:    image,
				OperatorTimestamp: operator,
			},
		},
	}
}

// NewProtoSDPRequest constructs a backend message for a client side sdp request
func NewProtoSDPRequest(msg *SDPRequest) *teleopproto.BackendMessage {
	return &teleopproto.BackendMessage{
		Payload: &teleopproto.BackendMessage_SdpRequest{
			SdpRequest: &teleopproto.SDPRequest{
				Sdp:          msg.SDP,
				Status:       msg.Status,
				ConnectionId: msg.ConnectionID,
			},
		},
	}
}

// NewProtoVideoRequest constructs a backend message for a client side video request
func NewProtoVideoRequest(msg *VideoRequest) *teleopproto.BackendMessage {
	return &teleopproto.BackendMessage{
		Payload: &teleopproto.BackendMessage_VideoRequest{
			VideoRequest: &teleopproto.VideoRequest{
				ConnectionId: msg.ConnectionID,
				Camera:       msg.CameraID,
				Width:        int32(msg.Width),
				Height:       int32(msg.Height),
			},
		},
	}
}

// NewProtoICECandidate constructs a backend message for a client side ice candidates
func NewProtoICECandidate(msg *ICECandidate) *teleopproto.BackendMessage {
	return &teleopproto.BackendMessage{
		Payload: &teleopproto.BackendMessage_IceCandidate{
			IceCandidate: &teleopproto.ICECandidate{
				ConnectionId:  msg.ConnectionID,
				Candidate:     msg.Candidate,
				SdpMid:        msg.SDPMid,
				SdpMlineIndex: int32(msg.SDPMLineIndex),
			},
		},
	}
}

// NewProtoConfirmation constructs a backend message for a client side sdp confirmation messages
func NewProtoConfirmation(msg *SDPConfirmation) *teleopproto.BackendMessage {
	return &teleopproto.BackendMessage{
		Payload: &teleopproto.BackendMessage_SdpConfirmation{
			SdpConfirmation: &teleopproto.SDPConfirmation{
				ConnectionId: msg.ConnectionID,
				Connected:    msg.Connected,
			},
		},
	}
}

// NewFrameMessage constructs a backend message containing a joystick command.
func NewFrameMessage(frame *teleopproto.CompressedImage) *teleopproto.VehicleMessage {
	return &teleopproto.VehicleMessage{
		Payload: &teleopproto.VehicleMessage_Frame{
			Frame: frame,
		},
	}
}

// NewSDPRequestMessage constructs a backend message for sdp offer/answer requests
func NewSDPRequestMessage(req *teleopproto.SDPRequest) *teleopproto.VehicleMessage {
	return &teleopproto.VehicleMessage{
		Payload: &teleopproto.VehicleMessage_SdpRequest{
			SdpRequest: req,
		},
	}
}

// NewICECandidateMessage constructs a backend message for ICE candidate discovery
func NewICECandidateMessage(req *teleopproto.ICECandidate) *teleopproto.VehicleMessage {
	return &teleopproto.VehicleMessage{
		Payload: &teleopproto.VehicleMessage_IceCandidate{
			IceCandidate: req,
		},
	}
}

// NewJPEG constructs a compressed image from a byte slice containing a
// JPEG-encoded image. The timestamp is set to the current time.
func NewJPEG(b []byte, w, h int) *teleopproto.CompressedImage {
	return &teleopproto.CompressedImage{
		Width:     int32(w),
		Height:    int32(h),
		Content:   b,
		Encoding:  teleopproto.Encoding_JPEG,
		Timestamp: NewSystemTimestamp(time.Now()),
	}
}

// NewGPSMessage construts a backend message containing a GPS location.
func NewGPSMessage(gps *halproto.GPSTelemetry) *teleopproto.VehicleMessage {
	return &teleopproto.VehicleMessage{
		Payload: &teleopproto.VehicleMessage_Gps{
			Gps: gps,
		},
	}
}

// NewDockingObservation constructs a new backend message containing discovered stations
func NewDockingObservation(timestamp time.Time, stationIDs []uint64) *teleopproto.VehicleMessage {
	return &teleopproto.VehicleMessage{
		Payload: &teleopproto.VehicleMessage_DockingObservation{
			DockingObservation: &teleopproto.DockingObservation{
				StationIds: stationIDs,
				Timestamp: &coreproto.SystemTimestamp{
					Nanos: uint64(timestamp.Nanosecond()),
				},
			},
		},
	}
}

// NewDockingStatus constructs a vehicle message to let us know the docking status
func NewDockingStatus(status teleopproto.DockingStatus_Status, distanceX, distanceY, angle float32) *teleopproto.VehicleMessage {
	return &teleopproto.VehicleMessage{
		Payload: &teleopproto.VehicleMessage_DockingStatus{
			DockingStatus: &teleopproto.DockingStatus{
				Status:             status,
				RemainingDistanceX: distanceX,
				RemainingDistanceY: distanceY,
				RemainingAngle:     angle,
			},
		},
	}
}

// NewDockCommand constructs a backend message to dock with a specific station
func NewDockCommand(stationID uint64) *teleopproto.BackendMessage {
	return &teleopproto.BackendMessage{
		Payload: &teleopproto.BackendMessage_DockCommand{
			DockCommand: &teleopproto.DockCommand{
				StationId: stationID,
			},
		},
	}
}

// NewConfirmation constructs a vehicle message to let us know the confirmation status of the command
func NewConfirmation(status teleopproto.Confirmation_Status, commandID, failureReason string) *teleopproto.VehicleMessage {
	return &teleopproto.VehicleMessage{
		Payload: &teleopproto.VehicleMessage_Confirmation{
			Confirmation: &teleopproto.Confirmation{
				Status:        status,
				MessageId:     commandID,
				FailureReason: failureReason,
			},
		},
	}
}

// NewDetection constructs a vehicle message with a bounding box
func NewDetection(boundingBoxes []*perceptionproto.ObjectBoundingBox) *teleopproto.VehicleMessage {
	return &teleopproto.VehicleMessage{
		Payload: &teleopproto.VehicleMessage_Detection{
			Detection: &perceptionproto.CameraAlignedBoxDetection{
				SystemTime:    NewSystemTimestamp(time.Now()),
				BoundingBoxes: boundingBoxes,
			},
		},
	}
}

// NewDetection3d constructs a vehicle message with a 3d bounding box
func NewDetection3d(boundingBoxes []*perceptionproto.Object3DBoundingBox, convexHulls []*perceptionproto.Object3DConvexHull) *teleopproto.VehicleMessage {
	return &teleopproto.VehicleMessage{
		Payload: &teleopproto.VehicleMessage_Detection3D{
			Detection3D: &perceptionproto.CameraAligned3DBoxDetection{
				SystemTime:    NewSystemTimestamp(time.Now()),
				BoundingBoxes: boundingBoxes,
				ConvexHulls:   convexHulls,
			},
		},
	}
}

// NewGPSTelemetry construts a GPS telemetry message.
func NewGPSTelemetry(lat, lon float64) *halproto.GPSTelemetry {
	return &halproto.GPSTelemetry{
		Latitude:  lat,
		Longitude: lon,
		Timestamp: NewSystemTimestamp(time.Now()),
	}
}

// NewManifestMessage construts a backend message containing a GPS location.
func NewManifestMessage(manifest *teleopproto.Manifest) *teleopproto.VehicleMessage {
	return &teleopproto.VehicleMessage{
		Payload: &teleopproto.VehicleMessage_Manifest{
			Manifest: manifest,
		},
	}
}

// NewDevice constructs a device
func NewDevice(name string) *halproto.Device {
	return &halproto.Device{
		Name: name,
	}
}

// NewUserView converts a User data struct to a view
func NewUserView(user *data.User) *UserView {
	return &UserView{
		UserID:   user.UserID,
		UserName: user.UserName,
	}
}

// UserAuthFromView converts a UserAuth view to a data UserAuth
func UserAuthFromView(auth *UserAuth) *data.UserAuth {
	return &data.UserAuth{
		UserName: auth.UserName,
		Password: auth.Password,
	}
}
