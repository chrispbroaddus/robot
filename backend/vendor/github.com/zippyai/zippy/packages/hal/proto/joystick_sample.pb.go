// Code generated by protoc-gen-go. DO NOT EDIT.
// source: packages/hal/proto/joystick_sample.proto

package hal

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import core "github.com/zippyai/zippy/packages/core/proto"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

type JoystickSample struct {
	Device            *Device                 `protobuf:"bytes,1,opt,name=device" json:"device,omitempty"`
	SystemTimestamp   *core.SystemTimestamp   `protobuf:"bytes,2,opt,name=systemTimestamp" json:"systemTimestamp,omitempty"`
	HardwareTimestamp *core.HardwareTimestamp `protobuf:"bytes,3,opt,name=hardwareTimestamp" json:"hardwareTimestamp,omitempty"`
	Axis              []float32               `protobuf:"fixed32,4,rep,packed,name=axis" json:"axis,omitempty"`
}

func (m *JoystickSample) Reset()                    { *m = JoystickSample{} }
func (m *JoystickSample) String() string            { return proto.CompactTextString(m) }
func (*JoystickSample) ProtoMessage()               {}
func (*JoystickSample) Descriptor() ([]byte, []int) { return fileDescriptor7, []int{0} }

func (m *JoystickSample) GetDevice() *Device {
	if m != nil {
		return m.Device
	}
	return nil
}

func (m *JoystickSample) GetSystemTimestamp() *core.SystemTimestamp {
	if m != nil {
		return m.SystemTimestamp
	}
	return nil
}

func (m *JoystickSample) GetHardwareTimestamp() *core.HardwareTimestamp {
	if m != nil {
		return m.HardwareTimestamp
	}
	return nil
}

func (m *JoystickSample) GetAxis() []float32 {
	if m != nil {
		return m.Axis
	}
	return nil
}

func init() {
	proto.RegisterType((*JoystickSample)(nil), "hal.JoystickSample")
}

func init() { proto.RegisterFile("packages/hal/proto/joystick_sample.proto", fileDescriptor7) }

var fileDescriptor7 = []byte{
	// 213 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xe2, 0xd2, 0x28, 0x48, 0x4c, 0xce,
	0x4e, 0x4c, 0x4f, 0x2d, 0xd6, 0xcf, 0x48, 0xcc, 0xd1, 0x2f, 0x28, 0xca, 0x2f, 0xc9, 0xd7, 0xcf,
	0xca, 0xaf, 0x2c, 0x2e, 0xc9, 0x4c, 0xce, 0x8e, 0x2f, 0x4e, 0xcc, 0x2d, 0xc8, 0x49, 0xd5, 0x03,
	0x8b, 0x0a, 0x31, 0x67, 0x24, 0xe6, 0x48, 0xc9, 0x63, 0x51, 0x9e, 0x92, 0x5a, 0x96, 0x99, 0x0c,
	0x55, 0x25, 0xa5, 0x0c, 0x57, 0x90, 0x9c, 0x5f, 0x94, 0x0a, 0x55, 0x51, 0x92, 0x99, 0x9b, 0x5a,
	0x5c, 0x92, 0x98, 0x5b, 0x00, 0x51, 0xa4, 0x74, 0x91, 0x91, 0x8b, 0xcf, 0x0b, 0x6a, 0x49, 0x30,
	0xd8, 0x0e, 0x21, 0x65, 0x2e, 0x36, 0x88, 0x39, 0x12, 0x8c, 0x0a, 0x8c, 0x1a, 0xdc, 0x46, 0xdc,
	0x7a, 0x19, 0x89, 0x39, 0x7a, 0x2e, 0x60, 0xa1, 0x20, 0xa8, 0x94, 0x90, 0x3d, 0x17, 0x7f, 0x71,
	0x65, 0x71, 0x49, 0x6a, 0x6e, 0x08, 0xcc, 0x40, 0x09, 0x26, 0xb0, 0x6a, 0x51, 0x3d, 0x90, 0x6d,
	0x7a, 0xc1, 0xa8, 0x92, 0x41, 0xe8, 0xaa, 0x85, 0x5c, 0xb9, 0x04, 0x33, 0x12, 0x8b, 0x52, 0xca,
	0x13, 0x8b, 0x52, 0x11, 0x46, 0x30, 0x83, 0x8d, 0x10, 0x87, 0x18, 0xe1, 0x81, 0x2e, 0x1d, 0x84,
	0xa9, 0x43, 0x48, 0x88, 0x8b, 0x25, 0xb1, 0x22, 0xb3, 0x58, 0x82, 0x45, 0x81, 0x59, 0x83, 0x29,
	0x08, 0xcc, 0x4e, 0x62, 0x03, 0x7b, 0xcd, 0x18, 0x10, 0x00, 0x00, 0xff, 0xff, 0x5b, 0x40, 0x36,
	0x8c, 0x51, 0x01, 0x00, 0x00,
}