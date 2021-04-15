package teleop

import (
	"time"

	coreproto "github.com/zippyai/zippy/packages/core/proto"
	halproto "github.com/zippyai/zippy/packages/hal/proto"
	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

// the GPS epoch from which all vehicle times are measured
var gpsEpoch = time.Date(1980, 1, 6, 0, 0, 0, 0, time.UTC)

// TimeFromProto converts coreproto.SystemTimestamp to time.Time
func TimeFromProto(pb *coreproto.SystemTimestamp) time.Time {
	if pb == nil {
		return time.Time{}
	}
	return gpsEpoch.Add(time.Duration(pb.Nanos) * time.Nanosecond)
}

// LocationSampleFromProto converts halproto.GPSTelemetry to teleop.LocationSample
func LocationSampleFromProto(pb *halproto.GPSTelemetry) *LocationSample {
	return &LocationSample{
		Timestamp: TimeFromProto(pb.GetTimestamp()),
		Location:  *LocationFromProto(pb),
	}
}

// LocationFromProto converts halproto.GPSTelemetry to teleop.Location
func LocationFromProto(pb *halproto.GPSTelemetry) *Location {
	return &Location{
		Lat: pb.GetLatitude(),
		Lon: pb.GetLongitude(),
		Alt: pb.GetAltitude(),
	}
}

// CameraSampleFromProto converts teleopproto.CompressedImage to teleop.CameraSample
func CameraSampleFromProto(pb *teleopproto.CompressedImage) *CameraSample {
	if pb.Device == nil {
		return nil
	}

	return &CameraSample{
		Timestamp: TimeFromProto(pb.GetTimestamp()),
		Camera:    pb.GetDevice().GetName(),
		Image:     *ImageFromProto(pb),
	}
}

// ImageFromProto converts teleopproto.CompressedImage to teleop.Image
func ImageFromProto(pb *teleopproto.CompressedImage) *Image {
	return &Image{
		Width:   int(pb.GetWidth()),
		Height:  int(pb.GetHeight()),
		Content: pb.GetContent(),
	}
}

// CameraFromProto converts teleopproto.Camera to teleop.Camera
func CameraFromProto(pb *teleopproto.Camera) *Camera {
	return &Camera{
		ID:     pb.GetDevice().GetName(),
		Width:  int(pb.GetWidth()),
		Height: int(pb.GetHeight()),
	}
}

// SDPRequestFromProto converts teleopproto.SDPRequest to teleop.SDPRequest
func SDPRequestFromProto(pb *teleopproto.SDPRequest) *SDPRequest {
	return &SDPRequest{
		ConnectionID: pb.ConnectionId,
		Status:       pb.Status,
		SDP:          pb.Sdp,
	}
}

// SDPConfirmationFromProto converts teleopproto.SDPConfirmation to teleop.SDPConfirmation
func SDPConfirmationFromProto(pb *teleopproto.SDPConfirmation) *SDPConfirmation {
	return &SDPConfirmation{
		ConnectionID: pb.ConnectionId,
		Connected:    pb.Connected,
	}
}

// ICECandidateFromProto converts teleopproto.ICECandidate to teleop.ICECandidate
func ICECandidateFromProto(pb *teleopproto.ICECandidate) *ICECandidate {
	return &ICECandidate{
		ConnectionID:  pb.ConnectionId,
		Candidate:     pb.Candidate,
		SDPMid:        pb.SdpMid,
		SDPMLineIndex: int(pb.SdpMlineIndex),
	}
}
