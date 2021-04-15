// Code generated by protoc-gen-go. DO NOT EDIT.
// source: packages/hal/proto/simulator_camera_response.proto

package hal

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

type SimulatorCameraResponse struct {
	Image *SimulatorCameraOutput `protobuf:"bytes,1,opt,name=image" json:"image,omitempty"`
	Depth *SimulatorCameraOutput `protobuf:"bytes,2,opt,name=depth" json:"depth,omitempty"`
	Xyz   *SimulatorCameraOutput `protobuf:"bytes,3,opt,name=xyz" json:"xyz,omitempty"`
}

func (m *SimulatorCameraResponse) Reset()                    { *m = SimulatorCameraResponse{} }
func (m *SimulatorCameraResponse) String() string            { return proto.CompactTextString(m) }
func (*SimulatorCameraResponse) ProtoMessage()               {}
func (*SimulatorCameraResponse) Descriptor() ([]byte, []int) { return fileDescriptor12, []int{0} }

func (m *SimulatorCameraResponse) GetImage() *SimulatorCameraOutput {
	if m != nil {
		return m.Image
	}
	return nil
}

func (m *SimulatorCameraResponse) GetDepth() *SimulatorCameraOutput {
	if m != nil {
		return m.Depth
	}
	return nil
}

func (m *SimulatorCameraResponse) GetXyz() *SimulatorCameraOutput {
	if m != nil {
		return m.Xyz
	}
	return nil
}

func init() {
	proto.RegisterType((*SimulatorCameraResponse)(nil), "hal.SimulatorCameraResponse")
}

func init() {
	proto.RegisterFile("packages/hal/proto/simulator_camera_response.proto", fileDescriptor12)
}

var fileDescriptor12 = []byte{
	// 167 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xe2, 0x32, 0x2a, 0x48, 0x4c, 0xce,
	0x4e, 0x4c, 0x4f, 0x2d, 0xd6, 0xcf, 0x48, 0xcc, 0xd1, 0x2f, 0x28, 0xca, 0x2f, 0xc9, 0xd7, 0x2f,
	0xce, 0xcc, 0x2d, 0xcd, 0x49, 0x2c, 0xc9, 0x2f, 0x8a, 0x4f, 0x4e, 0xcc, 0x4d, 0x2d, 0x4a, 0x8c,
	0x2f, 0x4a, 0x2d, 0x2e, 0xc8, 0xcf, 0x2b, 0x4e, 0xd5, 0x03, 0xcb, 0x0b, 0x31, 0x67, 0x24, 0xe6,
	0x48, 0x19, 0x10, 0xa3, 0x31, 0xbf, 0xb4, 0xa4, 0xa0, 0xb4, 0x04, 0xa2, 0x4d, 0x69, 0x35, 0x23,
	0x97, 0x78, 0x30, 0x4c, 0x85, 0x33, 0x58, 0x41, 0x10, 0xd4, 0x60, 0x21, 0x03, 0x2e, 0xd6, 0xcc,
	0xdc, 0xc4, 0xf4, 0x54, 0x09, 0x46, 0x05, 0x46, 0x0d, 0x6e, 0x23, 0x29, 0xbd, 0x8c, 0xc4, 0x1c,
	0x3d, 0x34, 0xc5, 0xfe, 0x60, 0xc3, 0x82, 0x20, 0x0a, 0x41, 0x3a, 0x52, 0x52, 0x0b, 0x4a, 0x32,
	0x24, 0x98, 0x08, 0xeb, 0x00, 0x2b, 0x14, 0xd2, 0xe1, 0x62, 0xae, 0xa8, 0xac, 0x92, 0x60, 0x26,
	0xa8, 0x1e, 0xa4, 0x2c, 0x89, 0x0d, 0xec, 0x68, 0x63, 0x40, 0x00, 0x00, 0x00, 0xff, 0xff, 0x3d,
	0xda, 0xdf, 0x13, 0x21, 0x01, 0x00, 0x00,
}
