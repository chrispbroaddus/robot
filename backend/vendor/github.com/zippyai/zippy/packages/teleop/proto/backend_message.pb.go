// Code generated by protoc-gen-go. DO NOT EDIT.
// source: packages/teleop/proto/backend_message.proto

/*
Package teleop is a generated protocol buffer package.

It is generated from these files:
	packages/teleop/proto/backend_message.proto
	packages/teleop/proto/camera.proto
	packages/teleop/proto/connection_options.proto
	packages/teleop/proto/vehicle_message.proto
	packages/teleop/proto/webrtc.proto

It has these top-level messages:
	BackendMessage
	JoystickCommand
	TurnInPlaceCommand
	PointAndGoCommand
	PointAndGoAndTurnInPlaceCommand
	DockCommand
	StopCommand
	ExposureCommand
	ResetExposureCommand
	ZStageCommand
	Camera
	VideoSource
	ConnectionOptions
	VehicleMessage
	CompressedImage
	Manifest
	DockingObservation
	DockingStatus
	Confirmation
	BatteryLevel
	Status
	Posture
	VideoRequest
	SDPRequest
	SDPConfirmation
	ICECandidate
*/
package teleop

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import core "github.com/zippyai/zippy/packages/core/proto"
import _ "github.com/zippyai/zippy/packages/hal/proto"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// This is a compile-time assertion to ensure that this generated file
// is compatible with the proto package it is being compiled against.
// A compilation error at this line likely means your copy of the
// proto package needs to be updated.
const _ = proto.ProtoPackageIsVersion2 // please upgrade the proto package

// BackendMessage is the message sent from backend to vehicle. It is the top-
// level protobuf that encompasses the entirety of each websocket frames.
type BackendMessage struct {
	// / ID identifies this message and is used for confirmations
	Id string `protobuf:"bytes,1,opt,name=id" json:"id,omitempty"`
	// / Payload contains one of the possible messages exchanged between the
	// / vehicle and the backend
	//
	// Types that are valid to be assigned to Payload:
	//	*BackendMessage_PointAndGo
	//	*BackendMessage_Joystick
	//	*BackendMessage_VideoRequest
	//	*BackendMessage_SdpRequest
	//	*BackendMessage_SdpConfirmation
	//	*BackendMessage_IceCandidate
	//	*BackendMessage_DockCommand
	//	*BackendMessage_StopCommand
	//	*BackendMessage_Exposure
	//	*BackendMessage_ResetExposure
	//	*BackendMessage_ZStage
	//	*BackendMessage_TurnInPlace
	//	*BackendMessage_PointAndGoAndTurnInPlace
	Payload isBackendMessage_Payload `protobuf_oneof:"payload"`
}

func (m *BackendMessage) Reset()                    { *m = BackendMessage{} }
func (m *BackendMessage) String() string            { return proto.CompactTextString(m) }
func (*BackendMessage) ProtoMessage()               {}
func (*BackendMessage) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{0} }

type isBackendMessage_Payload interface {
	isBackendMessage_Payload()
}

type BackendMessage_PointAndGo struct {
	PointAndGo *PointAndGoCommand `protobuf:"bytes,10,opt,name=pointAndGo,oneof"`
}
type BackendMessage_Joystick struct {
	Joystick *JoystickCommand `protobuf:"bytes,20,opt,name=joystick,oneof"`
}
type BackendMessage_VideoRequest struct {
	VideoRequest *VideoRequest `protobuf:"bytes,30,opt,name=videoRequest,oneof"`
}
type BackendMessage_SdpRequest struct {
	SdpRequest *SDPRequest `protobuf:"bytes,40,opt,name=sdpRequest,oneof"`
}
type BackendMessage_SdpConfirmation struct {
	SdpConfirmation *SDPConfirmation `protobuf:"bytes,50,opt,name=sdpConfirmation,oneof"`
}
type BackendMessage_IceCandidate struct {
	IceCandidate *ICECandidate `protobuf:"bytes,60,opt,name=iceCandidate,oneof"`
}
type BackendMessage_DockCommand struct {
	DockCommand *DockCommand `protobuf:"bytes,70,opt,name=dock_command,json=dockCommand,oneof"`
}
type BackendMessage_StopCommand struct {
	StopCommand *StopCommand `protobuf:"bytes,80,opt,name=stop_command,json=stopCommand,oneof"`
}
type BackendMessage_Exposure struct {
	Exposure *ExposureCommand `protobuf:"bytes,90,opt,name=exposure,oneof"`
}
type BackendMessage_ResetExposure struct {
	ResetExposure *ResetExposureCommand `protobuf:"bytes,110,opt,name=reset_exposure,json=resetExposure,oneof"`
}
type BackendMessage_ZStage struct {
	ZStage *ZStageCommand `protobuf:"bytes,120,opt,name=z_stage,json=zStage,oneof"`
}
type BackendMessage_TurnInPlace struct {
	TurnInPlace *TurnInPlaceCommand `protobuf:"bytes,130,opt,name=turnInPlace,oneof"`
}
type BackendMessage_PointAndGoAndTurnInPlace struct {
	PointAndGoAndTurnInPlace *PointAndGoAndTurnInPlaceCommand `protobuf:"bytes,140,opt,name=pointAndGoAndTurnInPlace,oneof"`
}

func (*BackendMessage_PointAndGo) isBackendMessage_Payload()               {}
func (*BackendMessage_Joystick) isBackendMessage_Payload()                 {}
func (*BackendMessage_VideoRequest) isBackendMessage_Payload()             {}
func (*BackendMessage_SdpRequest) isBackendMessage_Payload()               {}
func (*BackendMessage_SdpConfirmation) isBackendMessage_Payload()          {}
func (*BackendMessage_IceCandidate) isBackendMessage_Payload()             {}
func (*BackendMessage_DockCommand) isBackendMessage_Payload()              {}
func (*BackendMessage_StopCommand) isBackendMessage_Payload()              {}
func (*BackendMessage_Exposure) isBackendMessage_Payload()                 {}
func (*BackendMessage_ResetExposure) isBackendMessage_Payload()            {}
func (*BackendMessage_ZStage) isBackendMessage_Payload()                   {}
func (*BackendMessage_TurnInPlace) isBackendMessage_Payload()              {}
func (*BackendMessage_PointAndGoAndTurnInPlace) isBackendMessage_Payload() {}

func (m *BackendMessage) GetPayload() isBackendMessage_Payload {
	if m != nil {
		return m.Payload
	}
	return nil
}

func (m *BackendMessage) GetId() string {
	if m != nil {
		return m.Id
	}
	return ""
}

func (m *BackendMessage) GetPointAndGo() *PointAndGoCommand {
	if x, ok := m.GetPayload().(*BackendMessage_PointAndGo); ok {
		return x.PointAndGo
	}
	return nil
}

func (m *BackendMessage) GetJoystick() *JoystickCommand {
	if x, ok := m.GetPayload().(*BackendMessage_Joystick); ok {
		return x.Joystick
	}
	return nil
}

func (m *BackendMessage) GetVideoRequest() *VideoRequest {
	if x, ok := m.GetPayload().(*BackendMessage_VideoRequest); ok {
		return x.VideoRequest
	}
	return nil
}

func (m *BackendMessage) GetSdpRequest() *SDPRequest {
	if x, ok := m.GetPayload().(*BackendMessage_SdpRequest); ok {
		return x.SdpRequest
	}
	return nil
}

func (m *BackendMessage) GetSdpConfirmation() *SDPConfirmation {
	if x, ok := m.GetPayload().(*BackendMessage_SdpConfirmation); ok {
		return x.SdpConfirmation
	}
	return nil
}

func (m *BackendMessage) GetIceCandidate() *ICECandidate {
	if x, ok := m.GetPayload().(*BackendMessage_IceCandidate); ok {
		return x.IceCandidate
	}
	return nil
}

func (m *BackendMessage) GetDockCommand() *DockCommand {
	if x, ok := m.GetPayload().(*BackendMessage_DockCommand); ok {
		return x.DockCommand
	}
	return nil
}

func (m *BackendMessage) GetStopCommand() *StopCommand {
	if x, ok := m.GetPayload().(*BackendMessage_StopCommand); ok {
		return x.StopCommand
	}
	return nil
}

func (m *BackendMessage) GetExposure() *ExposureCommand {
	if x, ok := m.GetPayload().(*BackendMessage_Exposure); ok {
		return x.Exposure
	}
	return nil
}

func (m *BackendMessage) GetResetExposure() *ResetExposureCommand {
	if x, ok := m.GetPayload().(*BackendMessage_ResetExposure); ok {
		return x.ResetExposure
	}
	return nil
}

func (m *BackendMessage) GetZStage() *ZStageCommand {
	if x, ok := m.GetPayload().(*BackendMessage_ZStage); ok {
		return x.ZStage
	}
	return nil
}

func (m *BackendMessage) GetTurnInPlace() *TurnInPlaceCommand {
	if x, ok := m.GetPayload().(*BackendMessage_TurnInPlace); ok {
		return x.TurnInPlace
	}
	return nil
}

func (m *BackendMessage) GetPointAndGoAndTurnInPlace() *PointAndGoAndTurnInPlaceCommand {
	if x, ok := m.GetPayload().(*BackendMessage_PointAndGoAndTurnInPlace); ok {
		return x.PointAndGoAndTurnInPlace
	}
	return nil
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*BackendMessage) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _BackendMessage_OneofMarshaler, _BackendMessage_OneofUnmarshaler, _BackendMessage_OneofSizer, []interface{}{
		(*BackendMessage_PointAndGo)(nil),
		(*BackendMessage_Joystick)(nil),
		(*BackendMessage_VideoRequest)(nil),
		(*BackendMessage_SdpRequest)(nil),
		(*BackendMessage_SdpConfirmation)(nil),
		(*BackendMessage_IceCandidate)(nil),
		(*BackendMessage_DockCommand)(nil),
		(*BackendMessage_StopCommand)(nil),
		(*BackendMessage_Exposure)(nil),
		(*BackendMessage_ResetExposure)(nil),
		(*BackendMessage_ZStage)(nil),
		(*BackendMessage_TurnInPlace)(nil),
		(*BackendMessage_PointAndGoAndTurnInPlace)(nil),
	}
}

func _BackendMessage_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*BackendMessage)
	// payload
	switch x := m.Payload.(type) {
	case *BackendMessage_PointAndGo:
		b.EncodeVarint(10<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.PointAndGo); err != nil {
			return err
		}
	case *BackendMessage_Joystick:
		b.EncodeVarint(20<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.Joystick); err != nil {
			return err
		}
	case *BackendMessage_VideoRequest:
		b.EncodeVarint(30<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.VideoRequest); err != nil {
			return err
		}
	case *BackendMessage_SdpRequest:
		b.EncodeVarint(40<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.SdpRequest); err != nil {
			return err
		}
	case *BackendMessage_SdpConfirmation:
		b.EncodeVarint(50<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.SdpConfirmation); err != nil {
			return err
		}
	case *BackendMessage_IceCandidate:
		b.EncodeVarint(60<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.IceCandidate); err != nil {
			return err
		}
	case *BackendMessage_DockCommand:
		b.EncodeVarint(70<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.DockCommand); err != nil {
			return err
		}
	case *BackendMessage_StopCommand:
		b.EncodeVarint(80<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.StopCommand); err != nil {
			return err
		}
	case *BackendMessage_Exposure:
		b.EncodeVarint(90<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.Exposure); err != nil {
			return err
		}
	case *BackendMessage_ResetExposure:
		b.EncodeVarint(110<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.ResetExposure); err != nil {
			return err
		}
	case *BackendMessage_ZStage:
		b.EncodeVarint(120<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.ZStage); err != nil {
			return err
		}
	case *BackendMessage_TurnInPlace:
		b.EncodeVarint(130<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.TurnInPlace); err != nil {
			return err
		}
	case *BackendMessage_PointAndGoAndTurnInPlace:
		b.EncodeVarint(140<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.PointAndGoAndTurnInPlace); err != nil {
			return err
		}
	case nil:
	default:
		return fmt.Errorf("BackendMessage.Payload has unexpected type %T", x)
	}
	return nil
}

func _BackendMessage_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*BackendMessage)
	switch tag {
	case 10: // payload.pointAndGo
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(PointAndGoCommand)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_PointAndGo{msg}
		return true, err
	case 20: // payload.joystick
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(JoystickCommand)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_Joystick{msg}
		return true, err
	case 30: // payload.videoRequest
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(VideoRequest)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_VideoRequest{msg}
		return true, err
	case 40: // payload.sdpRequest
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(SDPRequest)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_SdpRequest{msg}
		return true, err
	case 50: // payload.sdpConfirmation
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(SDPConfirmation)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_SdpConfirmation{msg}
		return true, err
	case 60: // payload.iceCandidate
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(ICECandidate)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_IceCandidate{msg}
		return true, err
	case 70: // payload.dock_command
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(DockCommand)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_DockCommand{msg}
		return true, err
	case 80: // payload.stop_command
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(StopCommand)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_StopCommand{msg}
		return true, err
	case 90: // payload.exposure
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(ExposureCommand)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_Exposure{msg}
		return true, err
	case 110: // payload.reset_exposure
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(ResetExposureCommand)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_ResetExposure{msg}
		return true, err
	case 120: // payload.z_stage
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(ZStageCommand)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_ZStage{msg}
		return true, err
	case 130: // payload.turnInPlace
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(TurnInPlaceCommand)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_TurnInPlace{msg}
		return true, err
	case 140: // payload.pointAndGoAndTurnInPlace
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(PointAndGoAndTurnInPlaceCommand)
		err := b.DecodeMessage(msg)
		m.Payload = &BackendMessage_PointAndGoAndTurnInPlace{msg}
		return true, err
	default:
		return false, nil
	}
}

func _BackendMessage_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*BackendMessage)
	// payload
	switch x := m.Payload.(type) {
	case *BackendMessage_PointAndGo:
		s := proto.Size(x.PointAndGo)
		n += proto.SizeVarint(10<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *BackendMessage_Joystick:
		s := proto.Size(x.Joystick)
		n += proto.SizeVarint(20<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *BackendMessage_VideoRequest:
		s := proto.Size(x.VideoRequest)
		n += proto.SizeVarint(30<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *BackendMessage_SdpRequest:
		s := proto.Size(x.SdpRequest)
		n += proto.SizeVarint(40<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *BackendMessage_SdpConfirmation:
		s := proto.Size(x.SdpConfirmation)
		n += proto.SizeVarint(50<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *BackendMessage_IceCandidate:
		s := proto.Size(x.IceCandidate)
		n += proto.SizeVarint(60<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *BackendMessage_DockCommand:
		s := proto.Size(x.DockCommand)
		n += proto.SizeVarint(70<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *BackendMessage_StopCommand:
		s := proto.Size(x.StopCommand)
		n += proto.SizeVarint(80<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *BackendMessage_Exposure:
		s := proto.Size(x.Exposure)
		n += proto.SizeVarint(90<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *BackendMessage_ResetExposure:
		s := proto.Size(x.ResetExposure)
		n += proto.SizeVarint(110<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *BackendMessage_ZStage:
		s := proto.Size(x.ZStage)
		n += proto.SizeVarint(120<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *BackendMessage_TurnInPlace:
		s := proto.Size(x.TurnInPlace)
		n += proto.SizeVarint(130<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *BackendMessage_PointAndGoAndTurnInPlace:
		s := proto.Size(x.PointAndGoAndTurnInPlace)
		n += proto.SizeVarint(140<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

// JoystickCommand tells the vehicle to apply a specified torque and turn rate.
type JoystickCommand struct {
	// The desired velocity of the vehicle in m/s in the current direction of
	// travel.
	LinearVelocity float64 `protobuf:"fixed64,10,opt,name=linearVelocity" json:"linearVelocity,omitempty"`
	// The desired curvature.
	Curvature float64 `protobuf:"fixed64,20,opt,name=curvature" json:"curvature,omitempty"`
}

func (m *JoystickCommand) Reset()                    { *m = JoystickCommand{} }
func (m *JoystickCommand) String() string            { return proto.CompactTextString(m) }
func (*JoystickCommand) ProtoMessage()               {}
func (*JoystickCommand) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{1} }

func (m *JoystickCommand) GetLinearVelocity() float64 {
	if m != nil {
		return m.LinearVelocity
	}
	return 0
}

func (m *JoystickCommand) GetCurvature() float64 {
	if m != nil {
		return m.Curvature
	}
	return 0
}

// TurnInPlaceCommand tells the vehicle to turn in place to a given angle in
// radians
type TurnInPlaceCommand struct {
	// Angle in radians to turn the vehicle
	AngleToTurnInRadians float64 `protobuf:"fixed64,10,opt,name=angleToTurnInRadians" json:"angleToTurnInRadians,omitempty"`
}

func (m *TurnInPlaceCommand) Reset()                    { *m = TurnInPlaceCommand{} }
func (m *TurnInPlaceCommand) String() string            { return proto.CompactTextString(m) }
func (*TurnInPlaceCommand) ProtoMessage()               {}
func (*TurnInPlaceCommand) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{2} }

func (m *TurnInPlaceCommand) GetAngleToTurnInRadians() float64 {
	if m != nil {
		return m.AngleToTurnInRadians
	}
	return 0
}

// PointAndGoCommand tells the vehicle to navigate with respect to a location
// specified in image coordinates.
type PointAndGoCommand struct {
	// operatorTimestamp is time at which the operator performed the gesture
	// that generated this command.
	OperatorTimestamp *core.SystemTimestamp `protobuf:"bytes,10,opt,name=operatorTimestamp" json:"operatorTimestamp,omitempty"`
	// imageTimestamp is the timestamp of the frame the operator clicked on to
	// generate this command.
	ImageTimestamp *core.SystemTimestamp `protobuf:"bytes,20,opt,name=imageTimestamp" json:"imageTimestamp,omitempty"`
	// imageX is the x-coordinate of the pixel the operator clicked on, as a
	// fraction of the image width, with zero on the left edge and increasing
	// left to right.
	ImageX float64 `protobuf:"fixed64,30,opt,name=imageX" json:"imageX,omitempty"`
	// imageY is the y-coordinate of the pixel the operator clicked on, as a
	// fraction of the image height, with zero at the top edge and increasing
	// top to bottom.
	ImageY float64 `protobuf:"fixed64,40,opt,name=imageY" json:"imageY,omitempty"`
	// sourceCamera is the camera that the user is seeing when they sent this request
	Camera string `protobuf:"bytes,50,opt,name=camera" json:"camera,omitempty"`
	// Command override will ignore any free space constraints and execute the
	// point and go command without any checks
	CommandOverrideFlag bool `protobuf:"varint,60,opt,name=commandOverrideFlag" json:"commandOverrideFlag,omitempty"`
}

func (m *PointAndGoCommand) Reset()                    { *m = PointAndGoCommand{} }
func (m *PointAndGoCommand) String() string            { return proto.CompactTextString(m) }
func (*PointAndGoCommand) ProtoMessage()               {}
func (*PointAndGoCommand) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{3} }

func (m *PointAndGoCommand) GetOperatorTimestamp() *core.SystemTimestamp {
	if m != nil {
		return m.OperatorTimestamp
	}
	return nil
}

func (m *PointAndGoCommand) GetImageTimestamp() *core.SystemTimestamp {
	if m != nil {
		return m.ImageTimestamp
	}
	return nil
}

func (m *PointAndGoCommand) GetImageX() float64 {
	if m != nil {
		return m.ImageX
	}
	return 0
}

func (m *PointAndGoCommand) GetImageY() float64 {
	if m != nil {
		return m.ImageY
	}
	return 0
}

func (m *PointAndGoCommand) GetCamera() string {
	if m != nil {
		return m.Camera
	}
	return ""
}

func (m *PointAndGoCommand) GetCommandOverrideFlag() bool {
	if m != nil {
		return m.CommandOverrideFlag
	}
	return false
}

// PointAndGoAndTurnInPlaceCommand is a combination of the point and go and turn
// in place command, which will signal to the vehicle to navigate to a location
// specified in image coordinates and then turn in place after navigating
type PointAndGoAndTurnInPlaceCommand struct {
	// The point and go command
	PointAndGoCommand *PointAndGoCommand `protobuf:"bytes,10,opt,name=pointAndGoCommand" json:"pointAndGoCommand,omitempty"`
	// The turn in place command
	TurninPlaceCommand *TurnInPlaceCommand `protobuf:"bytes,20,opt,name=turninPlaceCommand" json:"turninPlaceCommand,omitempty"`
}

func (m *PointAndGoAndTurnInPlaceCommand) Reset()                    { *m = PointAndGoAndTurnInPlaceCommand{} }
func (m *PointAndGoAndTurnInPlaceCommand) String() string            { return proto.CompactTextString(m) }
func (*PointAndGoAndTurnInPlaceCommand) ProtoMessage()               {}
func (*PointAndGoAndTurnInPlaceCommand) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{4} }

func (m *PointAndGoAndTurnInPlaceCommand) GetPointAndGoCommand() *PointAndGoCommand {
	if m != nil {
		return m.PointAndGoCommand
	}
	return nil
}

func (m *PointAndGoAndTurnInPlaceCommand) GetTurninPlaceCommand() *TurnInPlaceCommand {
	if m != nil {
		return m.TurninPlaceCommand
	}
	return nil
}

// DockCommand tells the vehicle to dock to a docking station
// specified by the station id.
type DockCommand struct {
	// Each station has a unique station_id[uint64]
	// See docking_station.proto for full info about docking stations
	StationId uint64 `protobuf:"varint,30,opt,name=station_id,json=stationId" json:"station_id,omitempty"`
}

func (m *DockCommand) Reset()                    { *m = DockCommand{} }
func (m *DockCommand) String() string            { return proto.CompactTextString(m) }
func (*DockCommand) ProtoMessage()               {}
func (*DockCommand) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{5} }

func (m *DockCommand) GetStationId() uint64 {
	if m != nil {
		return m.StationId
	}
	return 0
}

// StopCommand tells the vehicle to cancel the previous command and stop in motion.
type StopCommand struct {
}

func (m *StopCommand) Reset()                    { *m = StopCommand{} }
func (m *StopCommand) String() string            { return proto.CompactTextString(m) }
func (*StopCommand) ProtoMessage()               {}
func (*StopCommand) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{6} }

// CameraExposure is used to tell the vehicle to modify the exposure rates of a camera in a given region
type ExposureCommand struct {
	// ID of the camera to expose
	CameraId string `protobuf:"bytes,20,opt,name=camera_id,json=cameraId" json:"camera_id,omitempty"`
	// x of the middle of the ROI selected to expose
	CenterX float64 `protobuf:"fixed64,30,opt,name=centerX" json:"centerX,omitempty"`
	// y of the middle of the ROI selected to expose as a fraction of the image height
	CenterY float64 `protobuf:"fixed64,40,opt,name=centerY" json:"centerY,omitempty"`
	// radius of the circle selected to expose as a fraction of the image width
	Radius float64 `protobuf:"fixed64,50,opt,name=radius" json:"radius,omitempty"`
}

func (m *ExposureCommand) Reset()                    { *m = ExposureCommand{} }
func (m *ExposureCommand) String() string            { return proto.CompactTextString(m) }
func (*ExposureCommand) ProtoMessage()               {}
func (*ExposureCommand) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{7} }

func (m *ExposureCommand) GetCameraId() string {
	if m != nil {
		return m.CameraId
	}
	return ""
}

func (m *ExposureCommand) GetCenterX() float64 {
	if m != nil {
		return m.CenterX
	}
	return 0
}

func (m *ExposureCommand) GetCenterY() float64 {
	if m != nil {
		return m.CenterY
	}
	return 0
}

func (m *ExposureCommand) GetRadius() float64 {
	if m != nil {
		return m.Radius
	}
	return 0
}

// ResetExposureCommand is used to reset all of a cameras exposures
type ResetExposureCommand struct {
	// Id of the camera to reset
	CameraId string `protobuf:"bytes,10,opt,name=camera_id,json=cameraId" json:"camera_id,omitempty"`
}

func (m *ResetExposureCommand) Reset()                    { *m = ResetExposureCommand{} }
func (m *ResetExposureCommand) String() string            { return proto.CompactTextString(m) }
func (*ResetExposureCommand) ProtoMessage()               {}
func (*ResetExposureCommand) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{8} }

func (m *ResetExposureCommand) GetCameraId() string {
	if m != nil {
		return m.CameraId
	}
	return ""
}

// ZStageCommand is used to lift the vehicle using the z stage
// z stage commands will move to the position declared at a speed of 0.01 meters/sec
// currently either position or left and right are set. Left and right only exist for future implementation
type ZStageCommand struct {
	// position should be between 0.0 and 0.40 representing height in meters
	Position float32 `protobuf:"fixed32,10,opt,name=position" json:"position,omitempty"`
}

func (m *ZStageCommand) Reset()                    { *m = ZStageCommand{} }
func (m *ZStageCommand) String() string            { return proto.CompactTextString(m) }
func (*ZStageCommand) ProtoMessage()               {}
func (*ZStageCommand) Descriptor() ([]byte, []int) { return fileDescriptor0, []int{9} }

func (m *ZStageCommand) GetPosition() float32 {
	if m != nil {
		return m.Position
	}
	return 0
}

func init() {
	proto.RegisterType((*BackendMessage)(nil), "teleop.BackendMessage")
	proto.RegisterType((*JoystickCommand)(nil), "teleop.JoystickCommand")
	proto.RegisterType((*TurnInPlaceCommand)(nil), "teleop.TurnInPlaceCommand")
	proto.RegisterType((*PointAndGoCommand)(nil), "teleop.PointAndGoCommand")
	proto.RegisterType((*PointAndGoAndTurnInPlaceCommand)(nil), "teleop.PointAndGoAndTurnInPlaceCommand")
	proto.RegisterType((*DockCommand)(nil), "teleop.DockCommand")
	proto.RegisterType((*StopCommand)(nil), "teleop.StopCommand")
	proto.RegisterType((*ExposureCommand)(nil), "teleop.ExposureCommand")
	proto.RegisterType((*ResetExposureCommand)(nil), "teleop.ResetExposureCommand")
	proto.RegisterType((*ZStageCommand)(nil), "teleop.ZStageCommand")
}

func init() { proto.RegisterFile("packages/teleop/proto/backend_message.proto", fileDescriptor0) }

var fileDescriptor0 = []byte{
	// 794 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x84, 0x95, 0xdd, 0x4e, 0x2b, 0x37,
	0x10, 0xc7, 0xd9, 0xa8, 0x0a, 0xc9, 0x04, 0x82, 0x30, 0xa1, 0xdd, 0xa6, 0xb4, 0x45, 0x5b, 0xa9,
	0x45, 0xa2, 0x0a, 0x08, 0x5a, 0xa9, 0xea, 0x87, 0x2a, 0x08, 0x1f, 0x09, 0x52, 0xd5, 0xc8, 0x41,
	0xb4, 0x70, 0x13, 0x99, 0xb5, 0x49, 0xdd, 0xec, 0xae, 0xb7, 0xb6, 0x43, 0x09, 0x97, 0xbd, 0xee,
	0xcd, 0x79, 0x99, 0xf3, 0x3a, 0xe7, 0x55, 0x8e, 0xd6, 0xfb, 0x9d, 0xe4, 0xc0, 0xe5, 0xfc, 0xe7,
	0xff, 0x9b, 0x4c, 0x6c, 0xcf, 0x2c, 0xec, 0x87, 0xc4, 0x9d, 0x90, 0x31, 0x53, 0x07, 0x9a, 0x79,
	0x4c, 0x84, 0x07, 0xa1, 0x14, 0x5a, 0x1c, 0xdc, 0x13, 0x77, 0xc2, 0x02, 0x3a, 0xf2, 0x99, 0x52,
	0x64, 0xcc, 0x3a, 0x46, 0x45, 0xd5, 0xd8, 0xd3, 0xfe, 0x2a, 0x83, 0x5c, 0x21, 0x59, 0x82, 0x68,
	0xee, 0x33, 0xa5, 0x89, 0x1f, 0xc6, 0xe6, 0xb6, 0x93, 0x99, 0xfe, 0x22, 0x5e, 0xe2, 0x71, 0x89,
	0xcf, 0x24, 0x19, 0x71, 0xba, 0xe0, 0x29, 0xfd, 0xfa, 0xbf, 0xec, 0x5e, 0x6a, 0x37, 0xf6, 0x38,
	0xef, 0xaa, 0xd0, 0x3c, 0x8d, 0xdb, 0xf9, 0x2d, 0xee, 0x06, 0x35, 0xa1, 0xc2, 0xa9, 0x6d, 0xed,
	0x5a, 0x7b, 0x75, 0x5c, 0xe1, 0x14, 0xfd, 0x04, 0x10, 0x0a, 0x1e, 0xe8, 0x93, 0x80, 0x5e, 0x0a,
	0x1b, 0x76, 0xad, 0xbd, 0xc6, 0xd1, 0xa7, 0x9d, 0xb8, 0x64, 0x67, 0x90, 0x65, 0xba, 0xc2, 0xf7,
	0x49, 0x40, 0x7b, 0x2b, 0xb8, 0x60, 0x47, 0xdf, 0x43, 0xed, 0x6f, 0x31, 0x53, 0x9a, 0xbb, 0x13,
	0xbb, 0x65, 0xd0, 0x4f, 0x52, 0xf4, 0x2a, 0xd1, 0x73, 0x30, 0xb3, 0xa2, 0x1f, 0x61, 0xed, 0x91,
	0x53, 0x26, 0x30, 0xfb, 0x67, 0xca, 0x94, 0xb6, 0xbf, 0x30, 0x68, 0x2b, 0x45, 0x6f, 0x0a, 0xb9,
	0xde, 0x0a, 0x2e, 0x79, 0xd1, 0x77, 0x00, 0x8a, 0x86, 0x29, 0xb9, 0x67, 0x48, 0x94, 0x92, 0xc3,
	0xb3, 0x41, 0xce, 0x15, 0x7c, 0xa8, 0x0b, 0x1b, 0x8a, 0x86, 0x5d, 0x11, 0x3c, 0x70, 0xe9, 0x13,
	0xcd, 0x45, 0x60, 0x1f, 0x95, 0xfb, 0x1d, 0x9e, 0x0d, 0x8a, 0xe9, 0xde, 0x0a, 0x9e, 0x27, 0xa2,
	0xb6, 0xb9, 0xcb, 0xba, 0x24, 0xa0, 0x9c, 0x12, 0xcd, 0xec, 0x9f, 0xcb, 0x6d, 0xf7, 0xbb, 0xe7,
	0x59, 0x2e, 0x6a, 0xbb, 0xe8, 0x45, 0x3f, 0xc0, 0x1a, 0x15, 0xee, 0x64, 0xe4, 0xc6, 0xc7, 0x61,
	0x5f, 0x18, 0x76, 0x2b, 0x65, 0xcf, 0x44, 0xf1, 0xa4, 0x1a, 0x34, 0x0f, 0x23, 0x52, 0x69, 0x11,
	0x66, 0xe4, 0xa0, 0x4c, 0x0e, 0xb5, 0x08, 0x0b, 0xa4, 0xca, 0xc3, 0xe8, 0x76, 0xd8, 0x53, 0x28,
	0xd4, 0x54, 0x32, 0xfb, 0xae, 0xfc, 0x6f, 0xcf, 0x13, 0xbd, 0x70, 0x3b, 0xa9, 0x15, 0x9d, 0x43,
	0x53, 0x32, 0xc5, 0xf4, 0x28, 0x83, 0x03, 0x03, 0xef, 0xa4, 0x30, 0x8e, 0xb2, 0x8b, 0x15, 0xd6,
	0x65, 0x51, 0x47, 0x87, 0xb0, 0xfa, 0x3c, 0x52, 0x9a, 0x8c, 0x99, 0xfd, 0x64, 0xf8, 0xed, 0x94,
	0xbf, 0x1b, 0x46, 0x6a, 0x0e, 0x56, 0x9f, 0x8d, 0x80, 0x7e, 0x85, 0x86, 0x9e, 0xca, 0xa0, 0x1f,
	0x0c, 0x3c, 0xe2, 0x32, 0xfb, 0x3f, 0xcb, 0x60, 0xed, 0x14, 0xbb, 0xce, 0x73, 0x85, 0x3f, 0x5c,
	0x20, 0xd0, 0x03, 0xd8, 0xf9, 0xe3, 0x3c, 0x09, 0x68, 0x81, 0xb0, 0xff, 0x8f, 0xab, 0x7d, 0xb3,
	0xf8, 0xb4, 0xcb, 0xc6, 0xbc, 0xf4, 0x07, 0x6b, 0x9d, 0xd6, 0x61, 0x35, 0x24, 0x33, 0x4f, 0x10,
	0xea, 0xfc, 0x01, 0x1b, 0x73, 0x2f, 0x1d, 0x7d, 0x0d, 0x4d, 0x8f, 0x07, 0x8c, 0xc8, 0x1b, 0xe6,
	0x09, 0x97, 0xeb, 0x99, 0x99, 0x2a, 0x0b, 0xcf, 0xa9, 0x68, 0x07, 0xea, 0xee, 0x54, 0x3e, 0x12,
	0x1d, 0x1d, 0x71, 0xcb, 0x58, 0x72, 0xc1, 0xe9, 0x01, 0x5a, 0xec, 0x0a, 0x1d, 0x41, 0x8b, 0x04,
	0x63, 0x8f, 0x5d, 0x8b, 0x38, 0x89, 0x09, 0xe5, 0x24, 0x50, 0xc9, 0x2f, 0x2c, 0xcd, 0x39, 0x6f,
	0x2a, 0xb0, 0xb9, 0x30, 0xc8, 0xa8, 0x0b, 0x9b, 0x22, 0x64, 0x92, 0x68, 0x21, 0xaf, 0xd3, 0xed,
	0x93, 0x8c, 0xff, 0x76, 0x27, 0x5a, 0x4d, 0x9d, 0xe1, 0x4c, 0x69, 0xe6, 0x67, 0x49, 0xbc, 0xe8,
	0x47, 0xbf, 0x40, 0x93, 0xfb, 0x64, 0xcc, 0xf2, 0x0a, 0xad, 0x97, 0x2a, 0xcc, 0x99, 0xd1, 0xc7,
	0x50, 0x35, 0xca, 0x9f, 0x66, 0x03, 0x58, 0x38, 0x89, 0x32, 0xfd, 0xd6, 0xcc, 0x77, 0xaa, 0xdf,
	0x46, 0x7a, 0xbc, 0x05, 0xcd, 0xf0, 0xd6, 0x71, 0x12, 0xa1, 0x43, 0xd8, 0x4a, 0xa6, 0xe3, 0xf7,
	0x47, 0x26, 0x25, 0xa7, 0xec, 0xc2, 0x23, 0x63, 0x33, 0x9f, 0x35, 0xbc, 0x2c, 0xe5, 0xbc, 0xb5,
	0xe0, 0xcb, 0x57, 0x5e, 0x00, 0xba, 0x84, 0xcd, 0x70, 0xfe, 0xd8, 0x5e, 0x5d, 0x90, 0x78, 0x91,
	0x41, 0x57, 0x80, 0xa2, 0x57, 0xca, 0x4b, 0xe5, 0x93, 0x93, 0x7a, 0xe1, 0x75, 0xe3, 0x25, 0x94,
	0xf3, 0x2d, 0x34, 0x0a, 0xbb, 0x02, 0x7d, 0x0e, 0xa0, 0xb4, 0xd9, 0x4e, 0x23, 0x4e, 0xcd, 0x29,
	0x7e, 0x84, 0xeb, 0x89, 0xd2, 0xa7, 0xce, 0x3a, 0x34, 0x0a, 0xfb, 0xc1, 0x79, 0x86, 0x8d, 0xb9,
	0xb1, 0x45, 0x9f, 0x41, 0x3d, 0xfb, 0xb0, 0x98, 0x96, 0xea, 0xb8, 0x16, 0x0b, 0x7d, 0x8a, 0x6c,
	0x58, 0x75, 0x59, 0xa0, 0x99, 0x4c, 0x2f, 0x28, 0x0d, 0xf3, 0x4c, 0x7a, 0x45, 0x69, 0x18, 0xdd,
	0x91, 0x24, 0x94, 0x4f, 0x95, 0xb9, 0x23, 0x0b, 0x27, 0x91, 0x73, 0x0c, 0xad, 0x65, 0x7b, 0xa3,
	0xdc, 0x00, 0x94, 0x1b, 0x70, 0xf6, 0x61, 0xbd, 0xb4, 0x2c, 0x50, 0x1b, 0x6a, 0xa1, 0x50, 0xdc,
	0x2c, 0xf0, 0xc8, 0x5c, 0xc1, 0x59, 0x7c, 0x5f, 0x35, 0xdf, 0xbc, 0xe3, 0xf7, 0x01, 0x00, 0x00,
	0xff, 0xff, 0x0f, 0x7d, 0x11, 0x3a, 0x97, 0x07, 0x00, 0x00,
}
