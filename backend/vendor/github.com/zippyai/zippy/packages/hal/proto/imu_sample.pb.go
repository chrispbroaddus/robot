// Code generated by protoc-gen-go. DO NOT EDIT.
// source: packages/hal/proto/imu_sample.proto

package hal

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import core "github.com/zippyai/zippy/packages/core/proto"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

type IMUSample struct {
	Device            *Device                 `protobuf:"bytes,1,opt,name=device" json:"device,omitempty"`
	SystemTimestamp   *core.SystemTimestamp   `protobuf:"bytes,2,opt,name=systemTimestamp" json:"systemTimestamp,omitempty"`
	HardwareTimestamp *core.HardwareTimestamp `protobuf:"bytes,3,opt,name=hardwareTimestamp" json:"hardwareTimestamp,omitempty"`
	Gyro              []float64               `protobuf:"fixed64,4,rep,packed,name=gyro" json:"gyro,omitempty"`
	Accel             []float64               `protobuf:"fixed64,5,rep,packed,name=accel" json:"accel,omitempty"`
	Mag               []float64               `protobuf:"fixed64,6,rep,packed,name=mag" json:"mag,omitempty"`
}

func (m *IMUSample) Reset()                    { *m = IMUSample{} }
func (m *IMUSample) String() string            { return proto.CompactTextString(m) }
func (*IMUSample) ProtoMessage()               {}
func (*IMUSample) Descriptor() ([]byte, []int) { return fileDescriptor6, []int{0} }

func (m *IMUSample) GetDevice() *Device {
	if m != nil {
		return m.Device
	}
	return nil
}

func (m *IMUSample) GetSystemTimestamp() *core.SystemTimestamp {
	if m != nil {
		return m.SystemTimestamp
	}
	return nil
}

func (m *IMUSample) GetHardwareTimestamp() *core.HardwareTimestamp {
	if m != nil {
		return m.HardwareTimestamp
	}
	return nil
}

func (m *IMUSample) GetGyro() []float64 {
	if m != nil {
		return m.Gyro
	}
	return nil
}

func (m *IMUSample) GetAccel() []float64 {
	if m != nil {
		return m.Accel
	}
	return nil
}

func (m *IMUSample) GetMag() []float64 {
	if m != nil {
		return m.Mag
	}
	return nil
}

func init() {
	proto.RegisterType((*IMUSample)(nil), "hal.IMUSample")
}

func init() { proto.RegisterFile("packages/hal/proto/imu_sample.proto", fileDescriptor6) }

var fileDescriptor6 = []byte{
	// 234 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x6c, 0x8f, 0xc1, 0x4a, 0x03, 0x31,
	0x10, 0x86, 0x59, 0xb7, 0x5d, 0x70, 0x7a, 0x50, 0x07, 0xc5, 0xd0, 0x8b, 0xc5, 0x5e, 0x7a, 0xca,
	0x82, 0x3e, 0x80, 0x17, 0x05, 0x3d, 0x78, 0xd9, 0xea, 0x59, 0xc6, 0x74, 0xd8, 0x5d, 0x4c, 0xd8,
	0x25, 0x89, 0x4a, 0xdf, 0xdb, 0x07, 0x90, 0x4e, 0x56, 0x8a, 0xab, 0xb7, 0x3f, 0xff, 0xff, 0xe5,
	0x0b, 0x81, 0x65, 0x4f, 0xe6, 0x8d, 0x6a, 0x0e, 0x65, 0x43, 0xb6, 0xec, 0x7d, 0x17, 0xbb, 0xb2,
	0x75, 0xef, 0x2f, 0x81, 0x5c, 0x6f, 0x59, 0x4b, 0x81, 0x79, 0x43, 0x76, 0x7e, 0xf1, 0x0f, 0xb9,
	0xe1, 0x8f, 0xd6, 0x0c, 0xd4, 0x7c, 0xaf, 0x32, 0x9d, 0xe7, 0x81, 0x88, 0xad, 0xe3, 0x10, 0xc9,
	0xf5, 0x09, 0xba, 0xfc, 0xca, 0xe0, 0xf0, 0xe1, 0xf1, 0x79, 0x2d, 0x7a, 0x5c, 0x42, 0x91, 0x14,
	0x2a, 0x5b, 0x64, 0xab, 0xd9, 0xd5, 0x4c, 0x37, 0x64, 0xf5, 0xad, 0x54, 0xd5, 0x30, 0xe1, 0x0d,
	0x1c, 0x85, 0x6d, 0x88, 0xec, 0x9e, 0x7e, 0x5c, 0xea, 0x40, 0xe8, 0x33, 0xbd, 0x7b, 0x48, 0xaf,
	0x7f, 0x8f, 0xd5, 0x98, 0xc6, 0x3b, 0x38, 0x69, 0xc8, 0x6f, 0x3e, 0xc9, 0xf3, 0x5e, 0x91, 0x8b,
	0xe2, 0x3c, 0x29, 0xee, 0xc7, 0x73, 0xf5, 0xf7, 0x06, 0x22, 0x4c, 0xea, 0xad, 0xef, 0xd4, 0x64,
	0x91, 0xaf, 0xb2, 0x4a, 0x32, 0x9e, 0xc2, 0x94, 0x8c, 0x61, 0xab, 0xa6, 0x52, 0xa6, 0x03, 0x1e,
	0x43, 0xee, 0xa8, 0x56, 0x85, 0x74, 0xbb, 0xf8, 0x5a, 0xc8, 0xef, 0xaf, 0xbf, 0x03, 0x00, 0x00,
	0xff, 0xff, 0xf7, 0x8d, 0x68, 0xaa, 0x6f, 0x01, 0x00, 0x00,
}
