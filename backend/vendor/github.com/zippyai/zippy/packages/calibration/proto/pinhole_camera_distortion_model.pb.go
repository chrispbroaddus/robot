// Code generated by protoc-gen-go. DO NOT EDIT.
// source: packages/calibration/proto/pinhole_camera_distortion_model.proto

package calibration

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// / Should be empty but protoc doesn't seem to like empty structs
type PinholeCameraDistortionModel struct {
	// / Send with zero length sequence of dummy
	Dummy []float64 `protobuf:"fixed64,1,rep,packed,name=dummy" json:"dummy,omitempty"`
}

func (m *PinholeCameraDistortionModel) Reset()                    { *m = PinholeCameraDistortionModel{} }
func (m *PinholeCameraDistortionModel) String() string            { return proto.CompactTextString(m) }
func (*PinholeCameraDistortionModel) ProtoMessage()               {}
func (*PinholeCameraDistortionModel) Descriptor() ([]byte, []int) { return fileDescriptor4, []int{0} }

func (m *PinholeCameraDistortionModel) GetDummy() []float64 {
	if m != nil {
		return m.Dummy
	}
	return nil
}

func init() {
	proto.RegisterType((*PinholeCameraDistortionModel)(nil), "calibration.PinholeCameraDistortionModel")
}

func init() {
	proto.RegisterFile("packages/calibration/proto/pinhole_camera_distortion_model.proto", fileDescriptor4)
}

var fileDescriptor4 = []byte{
	// 128 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xe2, 0x72, 0x28, 0x48, 0x4c, 0xce,
	0x4e, 0x4c, 0x4f, 0x2d, 0xd6, 0x4f, 0x4e, 0xcc, 0xc9, 0x4c, 0x2a, 0x4a, 0x2c, 0xc9, 0xcc, 0xcf,
	0xd3, 0x2f, 0x28, 0xca, 0x2f, 0xc9, 0xd7, 0x2f, 0xc8, 0xcc, 0xcb, 0xc8, 0xcf, 0x49, 0x8d, 0x4f,
	0x4e, 0xcc, 0x4d, 0x2d, 0x4a, 0x8c, 0x4f, 0xc9, 0x2c, 0x2e, 0xc9, 0x2f, 0x02, 0xc9, 0xc7, 0xe7,
	0xe6, 0xa7, 0xa4, 0xe6, 0xe8, 0x81, 0x55, 0x09, 0x71, 0x23, 0x69, 0x54, 0x32, 0xe1, 0x92, 0x09,
	0x80, 0xe8, 0x72, 0x06, 0x6b, 0x72, 0x81, 0xeb, 0xf1, 0x05, 0x69, 0x11, 0x12, 0xe1, 0x62, 0x4d,
	0x29, 0xcd, 0xcd, 0xad, 0x94, 0x60, 0x54, 0x60, 0xd6, 0x60, 0x0c, 0x82, 0x70, 0x92, 0xd8, 0xc0,
	0x26, 0x19, 0x03, 0x02, 0x00, 0x00, 0xff, 0xff, 0xc5, 0x88, 0x88, 0xb4, 0x8d, 0x00, 0x00, 0x00,
}
