// Code generated by protoc-gen-go. DO NOT EDIT.
// source: packages/hal/proto/vcu_status_response.proto

package hal

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import core "github.com/zippyai/zippy/packages/core/proto"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// / VCU Boot levels.
type VCUBootLevelID int32

const (
	VCUBootLevelID_BootLevelInit      VCUBootLevelID = 0
	VCUBootLevelID_BootLevelMotors    VCUBootLevelID = 1
	VCUBootLevelID_BootLevelCameras   VCUBootLevelID = 2
	VCUBootLevelID_BootLevelMessaging VCUBootLevelID = 3
	VCUBootLevelID_BootLevelControl   VCUBootLevelID = 4
	VCUBootLevelID_BootLevelContactor VCUBootLevelID = 5
	VCUBootLevelID_BootLevelTimeSync  VCUBootLevelID = 6
	VCUBootLevelID_BootLevelDone      VCUBootLevelID = 7
)

var VCUBootLevelID_name = map[int32]string{
	0: "BootLevelInit",
	1: "BootLevelMotors",
	2: "BootLevelCameras",
	3: "BootLevelMessaging",
	4: "BootLevelControl",
	5: "BootLevelContactor",
	6: "BootLevelTimeSync",
	7: "BootLevelDone",
}
var VCUBootLevelID_value = map[string]int32{
	"BootLevelInit":      0,
	"BootLevelMotors":    1,
	"BootLevelCameras":   2,
	"BootLevelMessaging": 3,
	"BootLevelControl":   4,
	"BootLevelContactor": 5,
	"BootLevelTimeSync":  6,
	"BootLevelDone":      7,
}

func (x VCUBootLevelID) String() string {
	return proto.EnumName(VCUBootLevelID_name, int32(x))
}
func (VCUBootLevelID) EnumDescriptor() ([]byte, []int) { return fileDescriptor29, []int{0} }

// / VCU unknown status request.
type VCUStatusUnknown struct {
}

func (m *VCUStatusUnknown) Reset()                    { *m = VCUStatusUnknown{} }
func (m *VCUStatusUnknown) String() string            { return proto.CompactTextString(m) }
func (*VCUStatusUnknown) ProtoMessage()               {}
func (*VCUStatusUnknown) Descriptor() ([]byte, []int) { return fileDescriptor29, []int{0} }

// / VCU version status request.
type VCUStatusVersion struct {
	MajorNumber     uint32 `protobuf:"varint,1,opt,name=majorNumber" json:"majorNumber,omitempty"`
	MinorNumber     uint32 `protobuf:"varint,2,opt,name=minorNumber" json:"minorNumber,omitempty"`
	PatchNumber     uint32 `protobuf:"varint,3,opt,name=patchNumber" json:"patchNumber,omitempty"`
	CompilationDate string `protobuf:"bytes,4,opt,name=compilationDate" json:"compilationDate,omitempty"`
	CompilationTime string `protobuf:"bytes,5,opt,name=compilationTime" json:"compilationTime,omitempty"`
}

func (m *VCUStatusVersion) Reset()                    { *m = VCUStatusVersion{} }
func (m *VCUStatusVersion) String() string            { return proto.CompactTextString(m) }
func (*VCUStatusVersion) ProtoMessage()               {}
func (*VCUStatusVersion) Descriptor() ([]byte, []int) { return fileDescriptor29, []int{1} }

func (m *VCUStatusVersion) GetMajorNumber() uint32 {
	if m != nil {
		return m.MajorNumber
	}
	return 0
}

func (m *VCUStatusVersion) GetMinorNumber() uint32 {
	if m != nil {
		return m.MinorNumber
	}
	return 0
}

func (m *VCUStatusVersion) GetPatchNumber() uint32 {
	if m != nil {
		return m.PatchNumber
	}
	return 0
}

func (m *VCUStatusVersion) GetCompilationDate() string {
	if m != nil {
		return m.CompilationDate
	}
	return ""
}

func (m *VCUStatusVersion) GetCompilationTime() string {
	if m != nil {
		return m.CompilationTime
	}
	return ""
}

// / VCU boot status request.
type VCUStatusBoot struct {
	VCUBootLevelID uint32 `protobuf:"varint,1,opt,name=VCUBootLevelID" json:"VCUBootLevelID,omitempty"`
}

func (m *VCUStatusBoot) Reset()                    { *m = VCUStatusBoot{} }
func (m *VCUStatusBoot) String() string            { return proto.CompactTextString(m) }
func (*VCUStatusBoot) ProtoMessage()               {}
func (*VCUStatusBoot) Descriptor() ([]byte, []int) { return fileDescriptor29, []int{2} }

func (m *VCUStatusBoot) GetVCUBootLevelID() uint32 {
	if m != nil {
		return m.VCUBootLevelID
	}
	return 0
}

// / VCU response to a status request.
type VCUStatusResponse struct {
	// / VCU sends back *just* the sequence number of the status request to which it is responding.
	SequenceNumber uint64 `protobuf:"fixed64,1,opt,name=sequenceNumber" json:"sequenceNumber,omitempty"`
	// / VCU sends back the status ID to which it is responding.
	StatusID VCUStatusID `protobuf:"varint,2,opt,name=statusID,enum=hal.VCUStatusID" json:"statusID,omitempty"`
	// / VCU's time when response was generated
	Timestamp *core.SystemTimestamp `protobuf:"bytes,3,opt,name=timestamp" json:"timestamp,omitempty"`
	// / The status message
	//
	// Types that are valid to be assigned to Status:
	//	*VCUStatusResponse_UnknownResponse
	//	*VCUStatusResponse_VersionResponse
	//	*VCUStatusResponse_BootResponse
	Status isVCUStatusResponse_Status `protobuf_oneof:"status"`
}

func (m *VCUStatusResponse) Reset()                    { *m = VCUStatusResponse{} }
func (m *VCUStatusResponse) String() string            { return proto.CompactTextString(m) }
func (*VCUStatusResponse) ProtoMessage()               {}
func (*VCUStatusResponse) Descriptor() ([]byte, []int) { return fileDescriptor29, []int{3} }

type isVCUStatusResponse_Status interface {
	isVCUStatusResponse_Status()
}

type VCUStatusResponse_UnknownResponse struct {
	UnknownResponse *VCUStatusUnknown `protobuf:"bytes,4,opt,name=unknownResponse,oneof"`
}
type VCUStatusResponse_VersionResponse struct {
	VersionResponse *VCUStatusVersion `protobuf:"bytes,5,opt,name=versionResponse,oneof"`
}
type VCUStatusResponse_BootResponse struct {
	BootResponse *VCUStatusBoot `protobuf:"bytes,6,opt,name=bootResponse,oneof"`
}

func (*VCUStatusResponse_UnknownResponse) isVCUStatusResponse_Status() {}
func (*VCUStatusResponse_VersionResponse) isVCUStatusResponse_Status() {}
func (*VCUStatusResponse_BootResponse) isVCUStatusResponse_Status()    {}

func (m *VCUStatusResponse) GetStatus() isVCUStatusResponse_Status {
	if m != nil {
		return m.Status
	}
	return nil
}

func (m *VCUStatusResponse) GetSequenceNumber() uint64 {
	if m != nil {
		return m.SequenceNumber
	}
	return 0
}

func (m *VCUStatusResponse) GetStatusID() VCUStatusID {
	if m != nil {
		return m.StatusID
	}
	return VCUStatusID_Unknown
}

func (m *VCUStatusResponse) GetTimestamp() *core.SystemTimestamp {
	if m != nil {
		return m.Timestamp
	}
	return nil
}

func (m *VCUStatusResponse) GetUnknownResponse() *VCUStatusUnknown {
	if x, ok := m.GetStatus().(*VCUStatusResponse_UnknownResponse); ok {
		return x.UnknownResponse
	}
	return nil
}

func (m *VCUStatusResponse) GetVersionResponse() *VCUStatusVersion {
	if x, ok := m.GetStatus().(*VCUStatusResponse_VersionResponse); ok {
		return x.VersionResponse
	}
	return nil
}

func (m *VCUStatusResponse) GetBootResponse() *VCUStatusBoot {
	if x, ok := m.GetStatus().(*VCUStatusResponse_BootResponse); ok {
		return x.BootResponse
	}
	return nil
}

// XXX_OneofFuncs is for the internal use of the proto package.
func (*VCUStatusResponse) XXX_OneofFuncs() (func(msg proto.Message, b *proto.Buffer) error, func(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error), func(msg proto.Message) (n int), []interface{}) {
	return _VCUStatusResponse_OneofMarshaler, _VCUStatusResponse_OneofUnmarshaler, _VCUStatusResponse_OneofSizer, []interface{}{
		(*VCUStatusResponse_UnknownResponse)(nil),
		(*VCUStatusResponse_VersionResponse)(nil),
		(*VCUStatusResponse_BootResponse)(nil),
	}
}

func _VCUStatusResponse_OneofMarshaler(msg proto.Message, b *proto.Buffer) error {
	m := msg.(*VCUStatusResponse)
	// status
	switch x := m.Status.(type) {
	case *VCUStatusResponse_UnknownResponse:
		b.EncodeVarint(4<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.UnknownResponse); err != nil {
			return err
		}
	case *VCUStatusResponse_VersionResponse:
		b.EncodeVarint(5<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.VersionResponse); err != nil {
			return err
		}
	case *VCUStatusResponse_BootResponse:
		b.EncodeVarint(6<<3 | proto.WireBytes)
		if err := b.EncodeMessage(x.BootResponse); err != nil {
			return err
		}
	case nil:
	default:
		return fmt.Errorf("VCUStatusResponse.Status has unexpected type %T", x)
	}
	return nil
}

func _VCUStatusResponse_OneofUnmarshaler(msg proto.Message, tag, wire int, b *proto.Buffer) (bool, error) {
	m := msg.(*VCUStatusResponse)
	switch tag {
	case 4: // status.unknownResponse
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(VCUStatusUnknown)
		err := b.DecodeMessage(msg)
		m.Status = &VCUStatusResponse_UnknownResponse{msg}
		return true, err
	case 5: // status.versionResponse
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(VCUStatusVersion)
		err := b.DecodeMessage(msg)
		m.Status = &VCUStatusResponse_VersionResponse{msg}
		return true, err
	case 6: // status.bootResponse
		if wire != proto.WireBytes {
			return true, proto.ErrInternalBadWireType
		}
		msg := new(VCUStatusBoot)
		err := b.DecodeMessage(msg)
		m.Status = &VCUStatusResponse_BootResponse{msg}
		return true, err
	default:
		return false, nil
	}
}

func _VCUStatusResponse_OneofSizer(msg proto.Message) (n int) {
	m := msg.(*VCUStatusResponse)
	// status
	switch x := m.Status.(type) {
	case *VCUStatusResponse_UnknownResponse:
		s := proto.Size(x.UnknownResponse)
		n += proto.SizeVarint(4<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VCUStatusResponse_VersionResponse:
		s := proto.Size(x.VersionResponse)
		n += proto.SizeVarint(5<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case *VCUStatusResponse_BootResponse:
		s := proto.Size(x.BootResponse)
		n += proto.SizeVarint(6<<3 | proto.WireBytes)
		n += proto.SizeVarint(uint64(s))
		n += s
	case nil:
	default:
		panic(fmt.Sprintf("proto: unexpected type %T in oneof", x))
	}
	return n
}

func init() {
	proto.RegisterType((*VCUStatusUnknown)(nil), "hal.VCUStatusUnknown")
	proto.RegisterType((*VCUStatusVersion)(nil), "hal.VCUStatusVersion")
	proto.RegisterType((*VCUStatusBoot)(nil), "hal.VCUStatusBoot")
	proto.RegisterType((*VCUStatusResponse)(nil), "hal.VCUStatusResponse")
	proto.RegisterEnum("hal.VCUBootLevelID", VCUBootLevelID_name, VCUBootLevelID_value)
}

func init() { proto.RegisterFile("packages/hal/proto/vcu_status_response.proto", fileDescriptor29) }

var fileDescriptor29 = []byte{
	// 472 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x84, 0x93, 0xd1, 0x6e, 0xda, 0x30,
	0x14, 0x86, 0x09, 0x94, 0xac, 0x75, 0x47, 0x31, 0xde, 0x3a, 0xa1, 0x5e, 0x21, 0x26, 0x4d, 0x68,
	0xab, 0x82, 0xd4, 0x5e, 0x6c, 0xb7, 0x2b, 0xb9, 0x28, 0xd2, 0xb6, 0x8b, 0x50, 0xb8, 0xad, 0x4c,
	0x74, 0x04, 0x59, 0x13, 0x9f, 0xcc, 0x76, 0x98, 0xfa, 0x72, 0x7b, 0x82, 0x3d, 0xc8, 0x1e, 0x63,
	0xb2, 0x93, 0x1a, 0x92, 0x4d, 0xea, 0x1d, 0xfc, 0xe7, 0xfb, 0x4f, 0xec, 0xff, 0x4f, 0xc8, 0x65,
	0xce, 0xe3, 0x07, 0xbe, 0x01, 0x35, 0xdd, 0xf2, 0x74, 0x9a, 0x4b, 0xd4, 0x38, 0xdd, 0xc5, 0xc5,
	0xbd, 0xd2, 0x5c, 0x17, 0xea, 0x5e, 0x82, 0xca, 0x51, 0x28, 0x08, 0xec, 0x84, 0x75, 0xb6, 0x3c,
	0xbd, 0x78, 0xeb, 0x2c, 0x31, 0x4a, 0xa8, 0x3c, 0x3a, 0xc9, 0x40, 0x69, 0x9e, 0xe5, 0x25, 0x79,
	0xf1, 0xe1, 0xb9, 0xbd, 0x3f, 0x0a, 0x50, 0xba, 0x84, 0xc7, 0x8c, 0xd0, 0xd5, 0x6c, 0xb9, 0xb0,
	0xa3, 0xa5, 0x78, 0x10, 0xf8, 0x53, 0x8c, 0x7f, 0x7b, 0x07, 0xe2, 0x0a, 0xa4, 0x4a, 0x50, 0xb0,
	0x11, 0x39, 0xcd, 0xf8, 0x77, 0x94, 0xdf, 0x8a, 0x6c, 0x0d, 0x72, 0xe8, 0x8d, 0xbc, 0x49, 0x2f,
	0x3a, 0x94, 0x2c, 0x91, 0x08, 0x47, 0xb4, 0x2b, 0x62, 0x2f, 0x19, 0x22, 0xe7, 0x3a, 0xde, 0x56,
	0x44, 0xa7, 0x24, 0x0e, 0x24, 0x36, 0x21, 0xfd, 0x18, 0xb3, 0x3c, 0x49, 0xb9, 0x4e, 0x50, 0x84,
	0x5c, 0xc3, 0xf0, 0x68, 0xe4, 0x4d, 0x4e, 0xa2, 0xa6, 0xdc, 0x20, 0xef, 0x92, 0x0c, 0x86, 0xdd,
	0x7f, 0x48, 0x23, 0x8f, 0x3f, 0x92, 0x9e, 0xbb, 0xcd, 0x0d, 0xa2, 0x66, 0xef, 0xc8, 0xd9, 0x6a,
	0xb6, 0x34, 0x3f, 0xbf, 0xc0, 0x0e, 0xd2, 0x79, 0x58, 0xdd, 0xa6, 0xa1, 0x8e, 0xff, 0xb4, 0xc9,
	0xc0, 0x39, 0xa3, 0xaa, 0x0e, 0xe3, 0x56, 0x26, 0x42, 0x11, 0xc3, 0x41, 0x16, 0x7e, 0xd4, 0x50,
	0xd9, 0x25, 0x39, 0x2e, 0x13, 0x9f, 0x87, 0x36, 0x8b, 0xb3, 0x2b, 0x1a, 0x6c, 0x79, 0x1a, 0xb8,
	0x8d, 0xf3, 0x30, 0x72, 0x04, 0xbb, 0x26, 0x27, 0xae, 0x47, 0x1b, 0xcc, 0xe9, 0xd5, 0x79, 0x60,
	0x4a, 0x0e, 0x16, 0x8f, 0x4a, 0x43, 0x76, 0xf7, 0x34, 0x8c, 0xf6, 0x1c, 0xfb, 0x4c, 0xfa, 0x45,
	0xd9, 0xd9, 0xd3, 0xe9, 0x6c, 0x5a, 0xc6, 0x5a, 0x7b, 0x52, 0x55, 0xec, 0x6d, 0x2b, 0x6a, 0xf2,
	0x66, 0xc5, 0xae, 0x6c, 0xd8, 0xad, 0xe8, 0xfe, 0x6f, 0x45, 0xf5, 0x1a, 0x98, 0x15, 0x0d, 0x9e,
	0x7d, 0x22, 0x2f, 0xd7, 0x88, 0xda, 0xf9, 0x7d, 0xeb, 0x67, 0x75, 0xbf, 0xc9, 0xf5, 0xb6, 0x15,
	0xd5, 0xc8, 0x9b, 0x63, 0xe2, 0x97, 0x01, 0xbc, 0xff, 0xe5, 0x35, 0x3b, 0x61, 0x03, 0xd2, 0xdb,
	0xff, 0x15, 0x89, 0xa6, 0x2d, 0xf6, 0x8a, 0xf4, 0x9d, 0xf4, 0x15, 0x35, 0x4a, 0x45, 0x3d, 0xf6,
	0x9a, 0x50, 0x27, 0xce, 0x78, 0x06, 0x92, 0x2b, 0xda, 0x66, 0x6f, 0x08, 0xdb, 0xa3, 0xa0, 0x14,
	0xdf, 0x24, 0x62, 0x43, 0x3b, 0x75, 0x1a, 0x85, 0x96, 0x98, 0xd2, 0xa3, 0x1a, 0x6d, 0x54, 0x1e,
	0x6b, 0x94, 0xb4, 0xcb, 0xce, 0xc9, 0xc0, 0xe9, 0xa6, 0x81, 0xc5, 0xa3, 0x88, 0xa9, 0x5f, 0x3b,
	0x5a, 0x88, 0x02, 0xe8, 0x8b, 0xb5, 0x6f, 0x3f, 0xa7, 0xeb, 0xbf, 0x01, 0x00, 0x00, 0xff, 0xff,
	0x7a, 0x25, 0x93, 0x4c, 0xd5, 0x03, 0x00, 0x00,
}
