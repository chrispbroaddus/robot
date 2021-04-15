// Code generated by protoc-gen-go. DO NOT EDIT.
// source: packages/teleop/proto/vehicle_message.proto

package teleop

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import core "github.com/zippyai/zippy/packages/core/proto"
import hal2 "github.com/zippyai/zippy/packages/hal/proto"
import hal3 "github.com/zippyai/zippy/packages/hal/proto"
import hal1 "github.com/zippyai/zippy/packages/hal/proto"
import perception "github.com/zippyai/zippy/packages/perception/proto"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// / Encoding describes how a compressed image was encoded
type Encoding int32

const (
	Encoding_JPEG Encoding = 0
)

var Encoding_name = map[int32]string{
	0: "JPEG",
}
var Encoding_value = map[string]int32{
	"JPEG": 0,
}

func (x Encoding) String() string {
	return proto.EnumName(Encoding_name, int32(x))
}
func (Encoding) EnumDescriptor() ([]byte, []int) { return fileDescriptor3, []int{0} }

type DockingStatus_Status int32

const (
	DockingStatus_SUCCESS    DockingStatus_Status = 0
	DockingStatus_FAILURE    DockingStatus_Status = 1
	DockingStatus_INPROGRESS DockingStatus_Status = 2
)

var DockingStatus_Status_name = map[int32]string{
	0: "SUCCESS",
	1: "FAILURE",
	2: "INPROGRESS",
}
var DockingStatus_Status_value = map[string]int32{
	"SUCCESS":    0,
	"FAILURE":    1,
	"INPROGRESS": 2,
}

func (x DockingStatus_Status) String() string {
	return proto.EnumName(DockingStatus_Status_name, int32(x))
}
func (DockingStatus_Status) EnumDescriptor() ([]byte, []int) { return fileDescriptor3, []int{4, 0} }

// / The possible results of processing a command
type Confirmation_Status int32

const (
	Confirmation_SUCCESS Confirmation_Status = 0
	Confirmation_FAILURE Confirmation_Status = 1
)

var Confirmation_Status_name = map[int32]string{
	0: "SUCCESS",
	1: "FAILURE",
}
var Confirmation_Status_value = map[string]int32{
	"SUCCESS": 0,
	"FAILURE": 1,
}

func (x Confirmation_Status) String() string {
	return proto.EnumName(Confirmation_Status_name, int32(x))
}
func (Confirmation_Status) EnumDescriptor() ([]byte, []int) { return fileDescriptor3, []int{5, 0} }

// / VehicleMessage is the message sent from vehicle to backend. It is the top-
// / level protobuf that encompasses the entirety of each websocket frames.
type VehicleMessage struct {
	// Types that are valid to be assigned to Payload:
	//	*VehicleMessage_Manifest
	//	*VehicleMessage_Frame
	//	*VehicleMessage_Gps
	//	*VehicleMessage_SdpRequest
	//	*VehicleMessage_SdpComfirmation
	//	*VehicleMessage_IceCandidate
	//	*VehicleMessage_DockingObservation
	//	*VehicleMessage_DockingStatus
	//	*VehicleMessage_Confirmation
	//	*VehicleMessage_VehicleStatus
	//	*VehicleMessage_Detection
	//	*VehicleMessage_Detection3D
	Payload isVehicleMessage_Payload `protobuf_oneof:"payload"`
}

func (m *VehicleMessage) Reset()                    { *m = VehicleMessage{} }
func (m *VehicleMessage) String() string            { return proto.CompactTextString(m) }
func (*VehicleMessage) ProtoMessage()               {}
func (*VehicleMessage) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{0} }

type isVehicleMessage_Payload interface {
	isVehicleMessage_Payload()
}

type VehicleMessage_Manifest struct {
	Manifest *Manifest `protobuf:"bytes,10,opt,name=manifest,oneof"`
}
type VehicleMessage_Frame struct {
	Frame *CompressedImage `protobuf:"bytes,20,opt,name=frame,oneof"`
}
type VehicleMessage_Gps struct {
	Gps *hal2.GPSTelemetry `protobuf:"bytes,30,opt,name=gps,oneof"`
}
type VehicleMessage_SdpRequest struct {
	SdpRequest *SDPRequest `protobuf:"bytes,40,opt,name=sdpRequest,oneof"`
}
type VehicleMessage_SdpComfirmation struct {
	SdpComfirmation *SDPConfirmation `protobuf:"bytes,50,opt,name=sdpComfirmation,oneof"`
}
type VehicleMessage_IceCandidate struct {
	IceCandidate *ICECandidate `protobuf:"bytes,60,opt,name=iceCandidate,oneof"`
}
type VehicleMessage_DockingObservation struct {
	DockingObservation *DockingObservation `protobuf:"bytes,70,opt,name=docking_observation,json=dockingObservation,oneof"`
}
type VehicleMessage_DockingStatus struct {
	DockingStatus *DockingStatus `protobuf:"bytes,80,opt,name=docking_status,json=dockingStatus,oneof"`
}
type VehicleMessage_Confirmation struct {
	Confirmation *Confirmation `protobuf:"bytes,90,opt,name=confirmation,oneof"`
}
type VehicleMessage_VehicleStatus struct {
	VehicleStatus *Status `protobuf:"bytes,100,opt,name=vehicle_status,json=vehicleStatus,oneof"`
}
type VehicleMessage_Detection struct {
	Detection *perception.CameraAlignedBoxDetection `protobuf:"bytes,110,opt,name=detection,oneof"`
}
type VehicleMessage_Detection3D struct {
	Detection3D *perception.CameraAligned3DBoxDetection `protobuf:"bytes,120,opt,name=detection3d,oneof"`
}

func (*VehicleMessage_Manifest) isVehicleMessage_Payload()           {}
func (*VehicleMessage_Frame) isVehicleMessage_Payload()              {}
func (*VehicleMessage_Gps) isVehicleMessage_Payload()                {}
func (*VehicleMessage_SdpRequest) isVehicleMessage_Payload()         {}
func (*VehicleMessage_SdpComfirmation) isVehicleMessage_Payload()    {}
func (*VehicleMessage_IceCandidate) isVehicleMessage_Payload()       {}
func (*VehicleMessage_DockingObservation) isVehicleMessage_Payload() {}
func (*VehicleMessage_DockingStatus) isVehicleMessage_Payload()      {}
func (*VehicleMessage_Confirmation) isVehicleMessage_Payload()       {}
func (*VehicleMessage_VehicleStatus) isVehicleMessage_Payload()      {}
func (*VehicleMessage_Detection) isVehicleMessage_Payload()          {}
func (*VehicleMessage_Detection3D) isVehicleMessage_Payload()        {}

func (m *VehicleMessage) GetPayload() isVehicleMessage_Payload {
	if m != nil {
		return m.Payload
	}
	return nil
}

func (m *VehicleMessage) GetManifest() *Manifest {
	if x, ok := m.GetPayload().(*VehicleMessage_Manifest); ok {
		return x.Manifest
	}
	return nil
}

func (m *VehicleMessage) GetFrame() *CompressedImage {
	if x, ok := m.GetPayload().(*VehicleMessage_Frame); ok {
		return x.Frame
	}
	return nil
}

func (m *VehicleMessage) GetGps() *hal2.GPSTelemetry {
	if x, ok := m.GetPayload().(*VehicleMessage_Gps); ok {
		return x.Gps
	}
	return nil
}

func (m *VehicleMessage) GetSdpRequest() *SDPRequest {
	if x, ok := m.GetPayload().(*VehicleMessage_SdpRequest); ok {
		return x.SdpRequest
	}
	return nil
}

func (m *VehicleMessage) GetSdpComfirmation() *SDPConfirmation {
	if x, ok := m.GetPayload().(*VehicleMessage_SdpComfirmation); ok {
		return x.SdpComfirmation
	}
	return nil
}

func (m *VehicleMessage) GetIceCandidate() *ICECandidate {
	if x, ok := m.GetPayload().(*VehicleMessage_IceCandidate); ok {
		return x.IceCandidate
	}
	return nil
}

func (m *VehicleMessage) GetDockingObservation() *DockingObservation {
	if x, ok := m.GetPayload().(*VehicleMessage_DockingObservation); ok {
		return x.DockingObservation
	}
	return nil
}

func (m *VehicleMessage) GetDockingStatus() *DockingStatus {
	if x, ok := m.GetPayload().(*VehicleMessage_DockingStatus); ok {
		return x.DockingStatus
	}
	return nil
}

func (m *VehicleMessage) GetConfirmation() *Confirmation {
	if x, ok := m.GetPayload().(*VehicleMessage_Confirmation); ok {
		return x.Confirmation
	}
	return nil
}

func (m *VehicleMessage) GetVehicleStatus() *Status {
	if x, ok := m.GetPayload().(*VehicleMessage_VehicleStatus); ok {
		return x.VehicleStatus
	}
	return nil
}

func (m *VehicleMessage) GetDetection() *perception.CameraAlignedBoxDetection {
	if x, ok := m.GetPayload().(*VehicleMessage_Detection); ok {
		return x.Detection
	}
	return nil
}

func (m *VehicleMessage) GetDetection3D() *perception.CameraAligned3DBoxDetection {
	if x, ok := m.GetPayload().(*VehicleMessage_Detection3D); ok {
		return x.Detection3D
	}
	return nil
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*VehicleMessage) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _VehicleMessage_OneofMarshaler, _VehicleMessage_OneofUnmarshaler, _VehicleMessage_OneofSizer, []interface{}{
		(*VehicleMessage_Manifest)(nil),
		(*VehicleMessage_Frame)(nil),
		(*VehicleMessage_Gps)(nil),
		(*VehicleMessage_SdpRequest)(nil),
		(*VehicleMessage_SdpComfirmation)(nil),
		(*VehicleMessage_IceCandidate)(nil),
		(*VehicleMessage_DockingObservation)(nil),
		(*VehicleMessage_DockingStatus)(nil),
		(*VehicleMessage_Confirmation)(nil),
		(*VehicleMessage_VehicleStatus)(nil),
		(*VehicleMessage_Detection)(nil),
		(*VehicleMessage_Detection3D)(nil),
	}
}

func _VehicleMessage_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*VehicleMessage)
	// payload
	switch x := m.Payload.(type) {
	case *VehicleMessage_Manifest:
		b.EncodeVarint(10<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.Manifest); err != nil {
			return err
		}
	case *VehicleMessage_Frame:
		b.EncodeVarint(20<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.Frame); err != nil {
			return err
		}
	case *VehicleMessage_Gps:
		b.EncodeVarint(30<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.Gps); err != nil {
			return err
		}
	case *VehicleMessage_SdpRequest:
		b.EncodeVarint(40<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.SdpRequest); err != nil {
			return err
		}
	case *VehicleMessage_SdpComfirmation:
		b.EncodeVarint(50<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.SdpComfirmation); err != nil {
			return err
		}
	case *VehicleMessage_IceCandidate:
		b.EncodeVarint(60<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.IceCandidate); err != nil {
			return err
		}
	case *VehicleMessage_DockingObservation:
		b.EncodeVarint(70<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.DockingObservation); err != nil {
			return err
		}
	case *VehicleMessage_DockingStatus:
		b.EncodeVarint(80<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.DockingStatus); err != nil {
			return err
		}
	case *VehicleMessage_Confirmation:
		b.EncodeVarint(90<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.Confirmation); err != nil {
			return err
		}
	case *VehicleMessage_VehicleStatus:
		b.EncodeVarint(100<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.VehicleStatus); err != nil {
			return err
		}
	case *VehicleMessage_Detection:
		b.EncodeVarint(110<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.Detection); err != nil {
			return err
		}
	case *VehicleMessage_Detection3D:
		b.EncodeVarint(120<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.Detection3D); err != nil {
			return err
		}
	case nil:
	default:
		return fmt.Errorf("VehicleMessage.Payload has unexpected type %T", x)
	}
	return nil
}

func _VehicleMessage_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*VehicleMessage)
	switch tag {
	case 10: // payload.manifest
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(Manifest)
		err := b.DecodeMessage(msg)
		m.Payload = &VehicleMessage_Manifest{msg}
		return true, err
	case 20: // payload.frame
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(CompressedImage)
		err := b.DecodeMessage(msg)
		m.Payload = &VehicleMessage_Frame{msg}
		return true, err
	case 30: // payload.gps
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(hal2.GPSTelemetry)
		err := b.DecodeMessage(msg)
		m.Payload = &VehicleMessage_Gps{msg}
		return true, err
	case 40: // payload.sdpRequest
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(SDPRequest)
		err := b.DecodeMessage(msg)
		m.Payload = &VehicleMessage_SdpRequest{msg}
		return true, err
	case 50: // payload.sdpComfirmation
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(SDPConfirmation)
		err := b.DecodeMessage(msg)
		m.Payload = &VehicleMessage_SdpComfirmation{msg}
		return true, err
	case 60: // payload.iceCandidate
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(ICECandidate)
		err := b.DecodeMessage(msg)
		m.Payload = &VehicleMessage_IceCandidate{msg}
		return true, err
	case 70: // payload.docking_observation
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(DockingObservation)
		err := b.DecodeMessage(msg)
		m.Payload = &VehicleMessage_DockingObservation{msg}
		return true, err
	case 80: // payload.docking_status
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(DockingStatus)
		err := b.DecodeMessage(msg)
		m.Payload = &VehicleMessage_DockingStatus{msg}
		return true, err
	case 90: // payload.confirmation
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(Confirmation)
		err := b.DecodeMessage(msg)
		m.Payload = &VehicleMessage_Confirmation{msg}
		return true, err
	case 100: // payload.vehicle_status
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(Status)
		err := b.DecodeMessage(msg)
		m.Payload = &VehicleMessage_VehicleStatus{msg}
		return true, err
	case 110: // payload.detection
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(perception.CameraAlignedBoxDetection)
		err := b.DecodeMessage(msg)
		m.Payload = &VehicleMessage_Detection{msg}
		return true, err
	case 120: // payload.detection3d
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(perception.CameraAligned3DBoxDetection)
		err := b.DecodeMessage(msg)
		m.Payload = &VehicleMessage_Detection3D{msg}
		return true, err
	default:
		return false, nil
	}
}

func _VehicleMessage_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*VehicleMessage)
	// payload
	switch x := m.Payload.(type) {
	case *VehicleMessage_Manifest:
		s := proto.Size(x.Manifest)
		n += proto.SizeVarint(10<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VehicleMessage_Frame:
		s := proto.Size(x.Frame)
		n += proto.SizeVarint(20<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VehicleMessage_Gps:
		s := proto.Size(x.Gps)
		n += proto.SizeVarint(30<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VehicleMessage_SdpRequest:
		s := proto.Size(x.SdpRequest)
		n += proto.SizeVarint(40<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VehicleMessage_SdpComfirmation:
		s := proto.Size(x.SdpComfirmation)
		n += proto.SizeVarint(50<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VehicleMessage_IceCandidate:
		s := proto.Size(x.IceCandidate)
		n += proto.SizeVarint(60<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VehicleMessage_DockingObservation:
		s := proto.Size(x.DockingObservation)
		n += proto.SizeVarint(70<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VehicleMessage_DockingStatus:
		s := proto.Size(x.DockingStatus)
		n += proto.SizeVarint(80<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VehicleMessage_Confirmation:
		s := proto.Size(x.Confirmation)
		n += proto.SizeVarint(90<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VehicleMessage_VehicleStatus:
		s := proto.Size(x.VehicleStatus)
		n += proto.SizeVarint(100<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VehicleMessage_Detection:
		s := proto.Size(x.Detection)
		n += proto.SizeVarint(110<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VehicleMessage_Detection3D:
		s := proto.Size(x.Detection3D)
		n += proto.SizeVarint(120<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

// / CompressedImage contains a compressed image.
type CompressedImage struct {
	// / Time when this image was captured
	Timestamp *core.SystemTimestamp `protobuf:"bytes,10,opt,name=timestamp" json:"timestamp,omitempty"`
	// / The device that generated this frame
	Device *hal1.Device `protobuf:"bytes,15,opt,name=device" json:"device,omitempty"`
	// / Width of the image in pixels
	Width int32 `protobuf:"varint,20,opt,name=width" json:"width,omitempty"`
	// / Height of the image in pixels
	Height int32 `protobuf:"varint,30,opt,name=height" json:"height,omitempty"`
	// / Encoded image
	Content []byte `protobuf:"bytes,40,opt,name=content,proto3" json:"content,omitempty"`
	// / Encoding used in the content blob above
	Encoding Encoding `protobuf:"varint,50,opt,name=encoding,enum=teleop.Encoding" json:"encoding,omitempty"`
}

func (m *CompressedImage) Reset()                    { *m = CompressedImage{} }
func (m *CompressedImage) String() string            { return proto.CompactTextString(m) }
func (*CompressedImage) ProtoMessage()               {}
func (*CompressedImage) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{1} }

func (m *CompressedImage) GetTimestamp() *core.SystemTimestamp {
	if m != nil {
		return m.Timestamp
	}
	return nil
}

func (m *CompressedImage) GetDevice() *hal1.Device {
	if m != nil {
		return m.Device
	}
	return nil
}

func (m *CompressedImage) GetWidth() int32 {
	if m != nil {
		return m.Width
	}
	return 0
}

func (m *CompressedImage) GetHeight() int32 {
	if m != nil {
		return m.Height
	}
	return 0
}

func (m *CompressedImage) GetContent() []byte {
	if m != nil {
		return m.Content
	}
	return nil
}

func (m *CompressedImage) GetEncoding() Encoding {
	if m != nil {
		return m.Encoding
	}
	return Encoding_JPEG
}

// / Manifest contains information about hardware present on a vehicle
type Manifest struct {
	// / Cameras attached to this vehicle
	Cameras []*Camera `protobuf:"bytes,20,rep,name=cameras" json:"cameras,omitempty"`
}

func (m *Manifest) Reset()                    { *m = Manifest{} }
func (m *Manifest) String() string            { return proto.CompactTextString(m) }
func (*Manifest) ProtoMessage()               {}
func (*Manifest) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{2} }

func (m *Manifest) GetCameras() []*Camera {
	if m != nil {
		return m.Cameras
	}
	return nil
}

// / DockingStations contains a list of available (dock-able) docking stations from the vehicle at the specific timestamp
type DockingObservation struct {
	// / System timestamp corresponding to hal.CameraSample.systemTimestamp
	Timestamp *core.SystemTimestamp `protobuf:"bytes,10,opt,name=timestamp" json:"timestamp,omitempty"`
	// / list of docking station ids
	StationIds []uint64 `protobuf:"varint,20,rep,packed,name=station_ids,json=stationIds" json:"station_ids,omitempty"`
}

func (m *DockingObservation) Reset()                    { *m = DockingObservation{} }
func (m *DockingObservation) String() string            { return proto.CompactTextString(m) }
func (*DockingObservation) ProtoMessage()               {}
func (*DockingObservation) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{3} }

func (m *DockingObservation) GetTimestamp() *core.SystemTimestamp {
	if m != nil {
		return m.Timestamp
	}
	return nil
}

func (m *DockingObservation) GetStationIds() []uint64 {
	if m != nil {
		return m.StationIds
	}
	return nil
}

// /
// / Docking status
// /
// / -----------O------------> (+x)
// /            |
// /            V (+y)
// /
// /               ^  ^
// /        ---    |t/
// /       /   --- |/
// /      /       -Q-
// /     /           ---
// /                    ---
// /                      /
// / O : anchor point on the target station
// / Q : anchor point on the vehicle
// /
// / remaining_distance_x&y : Position of Q in O coordinate, in meters. (i.e., in this example : positive x & y value)
// / remaining_angle : The angle t in the figure, in radian.
type DockingStatus struct {
	Status             DockingStatus_Status `protobuf:"varint,10,opt,name=status,enum=teleop.DockingStatus_Status" json:"status,omitempty"`
	RemainingDistanceX float32              `protobuf:"fixed32,20,opt,name=remaining_distance_x,json=remainingDistanceX" json:"remaining_distance_x,omitempty"`
	RemainingDistanceY float32              `protobuf:"fixed32,30,opt,name=remaining_distance_y,json=remainingDistanceY" json:"remaining_distance_y,omitempty"`
	RemainingAngle     float32              `protobuf:"fixed32,40,opt,name=remaining_angle,json=remainingAngle" json:"remaining_angle,omitempty"`
}

func (m *DockingStatus) Reset()                    { *m = DockingStatus{} }
func (m *DockingStatus) String() string            { return proto.CompactTextString(m) }
func (*DockingStatus) ProtoMessage()               {}
func (*DockingStatus) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{4} }

func (m *DockingStatus) GetStatus() DockingStatus_Status {
	if m != nil {
		return m.Status
	}
	return DockingStatus_SUCCESS
}

func (m *DockingStatus) GetRemainingDistanceX() float32 {
	if m != nil {
		return m.RemainingDistanceX
	}
	return 0
}

func (m *DockingStatus) GetRemainingDistanceY() float32 {
	if m != nil {
		return m.RemainingDistanceY
	}
	return 0
}

func (m *DockingStatus) GetRemainingAngle() float32 {
	if m != nil {
		return m.RemainingAngle
	}
	return 0
}

// / Confirmation is used to let the client know that a command was recieved and proccessed
// / by the vehicle or that it failed.
type Confirmation struct {
	// / The end result status of the command
	Status Confirmation_Status `protobuf:"varint,10,opt,name=status,enum=teleop.Confirmation_Status" json:"status,omitempty"`
	// / ID of the message that was sent to the vehicle
	MessageId string `protobuf:"bytes,20,opt,name=message_id,json=messageId" json:"message_id,omitempty"`
	// / The error messages as to why it failed is empty if successful
	FailureReason string `protobuf:"bytes,30,opt,name=failure_reason,json=failureReason" json:"failure_reason,omitempty"`
}

func (m *Confirmation) Reset()                    { *m = Confirmation{} }
func (m *Confirmation) String() string            { return proto.CompactTextString(m) }
func (*Confirmation) ProtoMessage()               {}
func (*Confirmation) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{5} }

func (m *Confirmation) GetStatus() Confirmation_Status {
	if m != nil {
		return m.Status
	}
	return Confirmation_SUCCESS
}

func (m *Confirmation) GetMessageId() string {
	if m != nil {
		return m.MessageId
	}
	return ""
}

func (m *Confirmation) GetFailureReason() string {
	if m != nil {
		return m.FailureReason
	}
	return ""
}

// BatteryLevel is used to let the client know where the battery's health is at
type BatteryLevel struct {
	// / percentage of the battery life that is remaining
	Remaining float32 `protobuf:"fixed32,10,opt,name=remaining" json:"remaining,omitempty"`
	// / current voltage state of the battery
	Voltage float32 `protobuf:"fixed32,20,opt,name=voltage" json:"voltage,omitempty"`
}

func (m *BatteryLevel) Reset()                    { *m = BatteryLevel{} }
func (m *BatteryLevel) String() string            { return proto.CompactTextString(m) }
func (*BatteryLevel) ProtoMessage()               {}
func (*BatteryLevel) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{6} }

func (m *BatteryLevel) GetRemaining() float32 {
	if m != nil {
		return m.Remaining
	}
	return 0
}

func (m *BatteryLevel) GetVoltage() float32 {
	if m != nil {
		return m.Voltage
	}
	return 0
}

// / Status messages are used to give insight into how the vehicle is operating
type Status struct {
	// Types that are valid to be assigned to Status:
	//	*Status_Posture
	//	*Status_BatteryLevel
	//	*Status_NetworkHealth
	Status isStatus_Status `protobuf_oneof:"status"`
}

func (m *Status) Reset()                    { *m = Status{} }
func (m *Status) String() string            { return proto.CompactTextString(m) }
func (*Status) ProtoMessage()               {}
func (*Status) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{7} }

type isStatus_Status interface {
	isStatus_Status()
}

type Status_Posture struct {
	Posture *Posture `protobuf:"bytes,10,opt,name=posture,oneof"`
}
type Status_BatteryLevel struct {
	BatteryLevel *BatteryLevel `protobuf:"bytes,20,opt,name=battery_level,json=batteryLevel,oneof"`
}
type Status_NetworkHealth struct {
	NetworkHealth *hal3.NetworkHealthTelemetry `protobuf:"bytes,30,opt,name=network_health,json=networkHealth,oneof"`
}

func (*Status_Posture) isStatus_Status()       {}
func (*Status_BatteryLevel) isStatus_Status()  {}
func (*Status_NetworkHealth) isStatus_Status() {}

func (m *Status) GetStatus() isStatus_Status {
	if m != nil {
		return m.Status
	}
	return nil
}

func (m *Status) GetPosture() *Posture {
	if x, ok := m.GetStatus().(*Status_Posture); ok {
		return x.Posture
	}
	return nil
}

func (m *Status) GetBatteryLevel() *BatteryLevel {
	if x, ok := m.GetStatus().(*Status_BatteryLevel); ok {
		return x.BatteryLevel
	}
	return nil
}

func (m *Status) GetNetworkHealth() *hal3.NetworkHealthTelemetry {
	if x, ok := m.GetStatus().(*Status_NetworkHealth); ok {
		return x.NetworkHealth
	}
	return nil
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*Status) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _Status_OneofMarshaler, _Status_OneofUnmarshaler, _Status_OneofSizer, []interface{}{
		(*Status_Posture)(nil),
		(*Status_BatteryLevel)(nil),
		(*Status_NetworkHealth)(nil),
	}
}

func _Status_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*Status)
	// status
	switch x := m.Status.(type) {
	case *Status_Posture:
		b.EncodeVarint(10<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.Posture); err != nil {
			return err
		}
	case *Status_BatteryLevel:
		b.EncodeVarint(20<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.BatteryLevel); err != nil {
			return err
		}
	case *Status_NetworkHealth:
		b.EncodeVarint(30<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.NetworkHealth); err != nil {
			return err
		}
	case nil:
	default:
		return fmt.Errorf("Status.Status has unexpected type %T", x)
	}
	return nil
}

func _Status_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*Status)
	switch tag {
	case 10: // status.posture
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(Posture)
		err := b.DecodeMessage(msg)
		m.Status = &Status_Posture{msg}
		return true, err
	case 20: // status.battery_level
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(BatteryLevel)
		err := b.DecodeMessage(msg)
		m.Status = &Status_BatteryLevel{msg}
		return true, err
	case 30: // status.network_health
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(hal3.NetworkHealthTelemetry)
		err := b.DecodeMessage(msg)
		m.Status = &Status_NetworkHealth{msg}
		return true, err
	default:
		return false, nil
	}
}

func _Status_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*Status)
	// status
	switch x := m.Status.(type) {
	case *Status_Posture:
		s := proto.Size(x.Posture)
		n += proto.SizeVarint(10<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *Status_BatteryLevel:
		s := proto.Size(x.BatteryLevel)
		n += proto.SizeVarint(20<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *Status_NetworkHealth:
		s := proto.Size(x.NetworkHealth)
		n += proto.SizeVarint(30<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

// / BodyPosition is used to tell the vehicles positioning irrespective of ground plane
type Posture struct {
	// / vehicles current slew in radians
	Slew float32 `protobuf:"fixed32,10,opt,name=slew" json:"slew,omitempty"`
	// / height of the right side of the zstage in meters from 0.0 to 0.4
	RightHeight float32 `protobuf:"fixed32,20,opt,name=right_height,json=rightHeight" json:"right_height,omitempty"`
	// / height of the left side of the zstage in meters from 0.0 to 0.4
	LeftHeight float32 `protobuf:"fixed32,30,opt,name=left_height,json=leftHeight" json:"left_height,omitempty"`
}

func (m *Posture) Reset()                    { *m = Posture{} }
func (m *Posture) String() string            { return proto.CompactTextString(m) }
func (*Posture) ProtoMessage()               {}
func (*Posture) Descriptor() ([]byte, []int) { return fileDescriptor3, []int{8} }

func (m *Posture) GetSlew() float32 {
	if m != nil {
		return m.Slew
	}
	return 0
}

func (m *Posture) GetRightHeight() float32 {
	if m != nil {
		return m.RightHeight
	}
	return 0
}

func (m *Posture) GetLeftHeight() float32 {
	if m != nil {
		return m.LeftHeight
	}
	return 0
}

func init() {
	proto.RegisterType((*VehicleMessage)(nil), "teleop.VehicleMessage")
	proto.RegisterType((*CompressedImage)(nil), "teleop.CompressedImage")
	proto.RegisterType((*Manifest)(nil), "teleop.Manifest")
	proto.RegisterType((*DockingObservation)(nil), "teleop.DockingObservation")
	proto.RegisterType((*DockingStatus)(nil), "teleop.DockingStatus")
	proto.RegisterType((*Confirmation)(nil), "teleop.Confirmation")
	proto.RegisterType((*BatteryLevel)(nil), "teleop.BatteryLevel")
	proto.RegisterType((*Status)(nil), "teleop.Status")
	proto.RegisterType((*Posture)(nil), "teleop.Posture")
	proto.RegisterEnum("teleop.Encoding", Encoding_name, Encoding_value)
	proto.RegisterEnum("teleop.DockingStatus_Status", DockingStatus_Status_name, DockingStatus_Status_value)
	proto.RegisterEnum("teleop.Confirmation_Status", Confirmation_Status_name, Confirmation_Status_value)
}

func init() { proto.RegisterFile("packages/teleop/proto/vehicle_message.proto", fileDescriptor3) }

var fileDescriptor3 = []byte{
	// 1018 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x9c, 0x55, 0xeb, 0x6e, 0x1b, 0x45,
	0x14, 0xde, 0xa4, 0x89, 0x2f, 0xc7, 0x89, 0x13, 0x06, 0x17, 0x56, 0x69, 0x21, 0x65, 0xab, 0xd0,
	0x40, 0x91, 0x03, 0x49, 0x24, 0x24, 0x40, 0x48, 0x89, 0xed, 0x64, 0x0d, 0x4d, 0x6b, 0x8d, 0x5b,
	0x04, 0xfc, 0xb1, 0x26, 0x3b, 0x27, 0xeb, 0x25, 0x7b, 0x63, 0x67, 0x72, 0x7b, 0x01, 0xde, 0x86,
	0x97, 0xe0, 0x4d, 0x78, 0x05, 0x9e, 0x00, 0xed, 0xcc, 0xec, 0xfa, 0x12, 0x17, 0xa4, 0xfe, 0xdb,
	0xf9, 0xce, 0x77, 0xce, 0x9c, 0x39, 0xf3, 0x7d, 0x3b, 0xf0, 0x3c, 0x65, 0xde, 0x25, 0xf3, 0x51,
	0xec, 0x49, 0x0c, 0x31, 0x49, 0xf7, 0xd2, 0x2c, 0x91, 0xc9, 0xde, 0x35, 0x8e, 0x03, 0x2f, 0xc4,
	0x51, 0x84, 0x42, 0x30, 0x1f, 0xdb, 0x0a, 0x25, 0x15, 0xcd, 0xd9, 0x7a, 0x5a, 0x26, 0x79, 0x49,
	0x86, 0x26, 0x45, 0x06, 0x11, 0x0a, 0xc9, 0xa2, 0x54, 0x93, 0xb7, 0x3e, 0x2d, 0x49, 0x63, 0x16,
	0x1a, 0x8e, 0x9f, 0x8a, 0x51, 0x5e, 0x23, 0x42, 0x99, 0xdd, 0x19, 0xde, 0x57, 0x0b, 0x78, 0x31,
	0xca, 0x9b, 0x24, 0xbb, 0x1c, 0x8d, 0x91, 0x85, 0x72, 0x7c, 0x2f, 0x65, 0x7b, 0x41, 0x0a, 0xc7,
	0xeb, 0xc0, 0x33, 0x8d, 0x6e, 0x7d, 0x56, 0x12, 0x52, 0xcc, 0x3c, 0x4c, 0x65, 0x90, 0xc4, 0x25,
	0x4f, 0xa2, 0x97, 0xaf, 0x0d, 0xd5, 0x59, 0x3c, 0x80, 0x1b, 0x3c, 0xcf, 0xa4, 0xf7, 0xdf, 0x1c,
	0x8f, 0x45, 0x98, 0x31, 0xcd, 0x71, 0xfe, 0x59, 0x85, 0xe6, 0x4f, 0x7a, 0x6a, 0x67, 0x7a, 0x68,
	0xa4, 0x0d, 0xb5, 0x88, 0xc5, 0xc1, 0x05, 0x0a, 0x69, 0xc3, 0x93, 0xa5, 0xdd, 0xc6, 0xfe, 0x66,
	0x5b, 0x17, 0x68, 0x9f, 0x19, 0xdc, 0xb5, 0x68, 0xc9, 0x21, 0x7b, 0xb0, 0x7a, 0x91, 0xb1, 0x08,
	0xed, 0x96, 0x22, 0x7f, 0x58, 0x90, 0x3b, 0x49, 0x94, 0x66, 0x28, 0x04, 0xf2, 0x7e, 0xc4, 0x7c,
	0x74, 0x2d, 0xaa, 0x79, 0x64, 0x07, 0x1e, 0xf8, 0xa9, 0xb0, 0x3f, 0x56, 0xf4, 0xf7, 0xda, 0x63,
	0x16, 0xb6, 0x4f, 0x07, 0xc3, 0xd7, 0xc5, 0xb4, 0x5c, 0x8b, 0xe6, 0x71, 0x72, 0x08, 0x20, 0x78,
	0x4a, 0xf1, 0xf7, 0xab, 0xbc, 0x93, 0x5d, 0xc5, 0x26, 0x45, 0xf1, 0x61, 0x77, 0x60, 0x22, 0xae,
	0x45, 0xa7, 0x78, 0xa4, 0x03, 0x1b, 0x82, 0xa7, 0x9d, 0x24, 0xba, 0x08, 0xb2, 0x88, 0xe5, 0x13,
	0xb3, 0xf7, 0x67, 0xfb, 0x1a, 0x76, 0x07, 0x9d, 0x24, 0x2e, 0xc3, 0xae, 0x45, 0xe7, 0x33, 0xc8,
	0x37, 0xb0, 0x16, 0x78, 0xd8, 0x61, 0x31, 0x0f, 0x38, 0x93, 0x68, 0x7f, 0xa7, 0x2a, 0xb4, 0x8a,
	0x0a, 0xfd, 0x4e, 0xaf, 0x8c, 0xb9, 0x16, 0x9d, 0xe1, 0x92, 0x33, 0x78, 0x9f, 0x27, 0xde, 0x65,
	0x10, 0xfb, 0xa3, 0xe4, 0x5c, 0x60, 0x76, 0xad, 0x9b, 0x38, 0x51, 0x25, 0xb6, 0x8a, 0x12, 0x5d,
	0x4d, 0x79, 0x35, 0x61, 0xb8, 0x16, 0x25, 0xfc, 0x1e, 0x4a, 0xbe, 0x87, 0x66, 0x51, 0x4e, 0x48,
	0x26, 0xaf, 0x84, 0x3d, 0x50, 0x95, 0x1e, 0xce, 0x55, 0x1a, 0xaa, 0xa0, 0x6b, 0xd1, 0x75, 0x3e,
	0x0d, 0xe4, 0x47, 0xf1, 0xa6, 0x4e, 0x6b, 0xff, 0x3a, 0x7b, 0x94, 0xb9, 0x49, 0xcc, 0x70, 0xc9,
	0xd7, 0xd0, 0x2c, 0x1c, 0x65, 0xf6, 0xe6, 0x2a, 0xbb, 0x59, 0x8e, 0xb2, 0xdc, 0xd4, 0xf0, 0xcc,
	0xa6, 0x3d, 0xa8, 0x97, 0x82, 0xb5, 0x63, 0x95, 0xb3, 0xd3, 0x9e, 0x68, 0xba, 0xdd, 0x51, 0x12,
	0x3c, 0x0a, 0x03, 0x3f, 0x46, 0x7e, 0x9c, 0xdc, 0x76, 0x0b, 0xb2, 0x6b, 0xd1, 0x49, 0x26, 0xf9,
	0x11, 0x1a, 0xe5, 0xe2, 0x80, 0xdb, 0xb7, 0xaa, 0xd0, 0xb3, 0xb7, 0x16, 0x3a, 0x98, 0x2f, 0x35,
	0x9d, 0x7d, 0x5c, 0x87, 0x6a, 0xca, 0xee, 0xc2, 0x84, 0x71, 0xe7, 0xef, 0x25, 0xd8, 0x98, 0x53,
	0x27, 0x39, 0x80, 0x7a, 0xf9, 0x2b, 0x30, 0xb2, 0x7f, 0xd8, 0xce, 0xff, 0x13, 0xed, 0xe1, 0x9d,
	0x90, 0x18, 0xbd, 0x2e, 0x82, 0x74, 0xc2, 0x23, 0x4f, 0xa1, 0xa2, 0x0d, 0x6c, 0x6f, 0xa8, 0x8c,
	0x86, 0x12, 0x73, 0x57, 0x41, 0xd4, 0x84, 0x48, 0x0b, 0x56, 0x6f, 0x02, 0x2e, 0xc7, 0xca, 0x1f,
	0xab, 0x54, 0x2f, 0xc8, 0x07, 0x50, 0x19, 0x63, 0xe0, 0x8f, 0xa5, 0xf2, 0xc1, 0x2a, 0x35, 0x2b,
	0x62, 0x43, 0xd5, 0x4b, 0x62, 0x89, 0xb1, 0x96, 0xfc, 0x1a, 0x2d, 0x96, 0xe4, 0x0b, 0xa8, 0x61,
	0xec, 0x25, 0x3c, 0x88, 0x7d, 0x25, 0xe9, 0xe6, 0xc4, 0x97, 0x3d, 0x83, 0xd3, 0x92, 0xe1, 0x1c,
	0x42, 0xad, 0x70, 0x2b, 0xd9, 0x85, 0xaa, 0x36, 0xbd, 0xb0, 0x5b, 0x4f, 0x1e, 0x4c, 0x5f, 0xa0,
	0x9e, 0x1f, 0x2d, 0xc2, 0xce, 0x6f, 0x40, 0xee, 0x2b, 0xf3, 0xdd, 0x66, 0xb3, 0x0d, 0x8d, 0x5c,
	0x34, 0x41, 0x12, 0x8f, 0x02, 0xae, 0x37, 0x5e, 0xa1, 0x60, 0xa0, 0x3e, 0x17, 0xce, 0x1f, 0xcb,
	0xb0, 0x3e, 0x23, 0x5e, 0x72, 0x08, 0x15, 0xa3, 0x33, 0x50, 0xe7, 0x7b, 0xbc, 0x50, 0xe3, 0x46,
	0x75, 0xd4, 0x70, 0xc9, 0x97, 0xd0, 0xca, 0x30, 0x62, 0x41, 0x9c, 0x7b, 0x84, 0x07, 0x42, 0xb2,
	0xd8, 0xc3, 0xd1, 0xad, 0x1a, 0xf7, 0x32, 0x25, 0x65, 0xac, 0x6b, 0x42, 0x3f, 0xbf, 0x25, 0xe3,
	0x4e, 0xdd, 0xc4, 0xa2, 0x8c, 0x5f, 0xc8, 0x33, 0xd8, 0x98, 0x64, 0xb0, 0xd8, 0x0f, 0x51, 0xdd,
	0xce, 0x32, 0x6d, 0x96, 0xf0, 0x51, 0x8e, 0x3a, 0xfb, 0x50, 0x31, 0x87, 0x69, 0x40, 0x75, 0xf8,
	0xa6, 0xd3, 0xe9, 0x0d, 0x87, 0x9b, 0x56, 0xbe, 0x38, 0x39, 0xea, 0xbf, 0x78, 0x43, 0x7b, 0x9b,
	0x4b, 0xa4, 0x09, 0xd0, 0x7f, 0x39, 0xa0, 0xaf, 0x4e, 0x69, 0x1e, 0x5c, 0x76, 0xfe, 0x5c, 0x82,
	0xb5, 0x69, 0x1f, 0x92, 0x83, 0xb9, 0x39, 0x3c, 0x5a, 0xe4, 0xd6, 0xf9, 0x31, 0x7c, 0x04, 0x60,
	0x9e, 0xbd, 0x51, 0xc0, 0xd5, 0xe1, 0xeb, 0xb4, 0x6e, 0x90, 0x3e, 0x27, 0x3b, 0xd0, 0xbc, 0x60,
	0x41, 0x78, 0x95, 0xe1, 0x28, 0x43, 0x26, 0x92, 0x58, 0x9d, 0xb6, 0x4e, 0xd7, 0x0d, 0x4a, 0x15,
	0xe8, 0x38, 0xff, 0xdf, 0xbf, 0x73, 0x02, 0x6b, 0xc7, 0x4c, 0x4a, 0xcc, 0xee, 0x5e, 0xe0, 0x35,
	0x86, 0xe4, 0x31, 0xd4, 0xcb, 0x29, 0xa8, 0x8e, 0x97, 0xe9, 0x04, 0xc8, 0x05, 0x7d, 0x9d, 0x84,
	0x92, 0xf9, 0x68, 0x6e, 0xa4, 0x58, 0x3a, 0x7f, 0x2d, 0x95, 0x9b, 0x3d, 0x87, 0x6a, 0x9a, 0x08,
	0x79, 0x95, 0xa1, 0xd1, 0xd7, 0x46, 0x71, 0xe4, 0x81, 0x86, 0x5d, 0x8b, 0x16, 0x0c, 0xf2, 0x2d,
	0xac, 0x9f, 0xeb, 0xfd, 0x47, 0x61, 0xde, 0x80, 0x79, 0x78, 0xca, 0x7f, 0xda, 0x74, 0x73, 0xf9,
	0x3f, 0xed, 0x7c, 0xba, 0xd9, 0x2e, 0x34, 0x67, 0x9f, 0x69, 0xf3, 0x0e, 0x3d, 0x52, 0xd6, 0x7d,
	0xa9, 0x43, 0xae, 0x8a, 0x4c, 0xbf, 0x48, 0xeb, 0xf1, 0x74, 0xe4, 0xb8, 0x56, 0xdc, 0x90, 0xc3,
	0xa0, 0x6a, 0x5a, 0x24, 0x04, 0x56, 0x44, 0x88, 0x37, 0x66, 0x04, 0xea, 0x9b, 0x7c, 0x02, 0x6b,
	0x59, 0xee, 0xeb, 0x91, 0x31, 0xbb, 0x1e, 0x41, 0x43, 0x61, 0xae, 0x76, 0xfc, 0x36, 0x34, 0x42,
	0xbc, 0x28, 0x19, 0x5a, 0x84, 0x90, 0x43, 0x9a, 0xf0, 0x79, 0x0b, 0x6a, 0x85, 0xc1, 0x49, 0x0d,
	0x56, 0x7e, 0x18, 0xf4, 0x4e, 0x37, 0xad, 0xf3, 0x8a, 0x7a, 0xc0, 0x0f, 0xfe, 0x0d, 0x00, 0x00,
	0xff, 0xff, 0x5b, 0x39, 0x56, 0x90, 0x0b, 0x09, 0x00, 0x00,
}