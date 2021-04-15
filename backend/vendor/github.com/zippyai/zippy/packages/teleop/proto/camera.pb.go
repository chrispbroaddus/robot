// Code generated by protoc-gen-go. DO NOT EDIT.
// source: packages/teleop/proto/camera.proto

package teleop

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import hal1 "github.com/zippyai/zippy/packages/hal/proto"
import hal "github.com/zippyai/zippy/packages/hal/proto"
import calibration2 "github.com/zippyai/zippy/packages/calibration/proto"
import calibration4 "github.com/zippyai/zippy/packages/calibration/proto"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// / Camera contains information about a physical camera on a vehicle
type Camera struct {
	// / The device for this camera
	Device *hal1.Device `protobuf:"bytes,10,opt,name=device" json:"device,omitempty"`
	// / Width of images generated by this camera
	Width int32 `protobuf:"varint,20,opt,name=width" json:"width,omitempty"`
	// / Height of images generated by this camera
	Height int32 `protobuf:"varint,30,opt,name=height" json:"height,omitempty"`
	// / Camera intrinsic calibration
	Intrinsics *calibration2.CameraIntrinsicCalibration `protobuf:"bytes,40,opt,name=intrinsics" json:"intrinsics,omitempty"`
	// / Transformation from the world coordinate frame, in which z=0
	// / corresponds to the ground plane and the positive X axis corresponds to
	// / the forwards direction of travel for the robot, to the camera
	// / coordinate frame, in which the positive Z axis corresponds to the
	// / principle ray.
	Extrinsics *calibration4.CoordinateTransformation `protobuf:"bytes,50,opt,name=extrinsics" json:"extrinsics,omitempty"`
	// Role that this camera is for
	Role hal.CameraId `protobuf:"varint,60,opt,name=role,enum=hal.CameraId" json:"role,omitempty"`
}

func (m *Camera) Reset()                    { *m = Camera{} }
func (m *Camera) String() string            { return proto.CompactTextString(m) }
func (*Camera) ProtoMessage()               {}
func (*Camera) Descriptor() ([]byte, []int) { return fileDescriptor1, []int{0} }

func (m *Camera) GetDevice() *hal1.Device {
	if m != nil {
		return m.Device
	}
	return nil
}

func (m *Camera) GetWidth() int32 {
	if m != nil {
		return m.Width
	}
	return 0
}

func (m *Camera) GetHeight() int32 {
	if m != nil {
		return m.Height
	}
	return 0
}

func (m *Camera) GetIntrinsics() *calibration2.CameraIntrinsicCalibration {
	if m != nil {
		return m.Intrinsics
	}
	return nil
}

func (m *Camera) GetExtrinsics() *calibration4.CoordinateTransformation {
	if m != nil {
		return m.Extrinsics
	}
	return nil
}

func (m *Camera) GetRole() hal.CameraId {
	if m != nil {
		return m.Role
	}
	return hal.CameraId_FrontLeftStereo
}

func init() {
	proto.RegisterType((*Camera)(nil), "teleop.Camera")
}

func init() { proto.RegisterFile("packages/teleop/proto/camera.proto", fileDescriptor1) }

var fileDescriptor1 = []byte{
	// 276 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x7c, 0x90, 0x41, 0x4b, 0xc4, 0x30,
	0x10, 0x85, 0x59, 0x71, 0x7b, 0x98, 0x45, 0x0f, 0x61, 0x91, 0xb2, 0x07, 0xad, 0x15, 0xb1, 0xa7,
	0x16, 0xea, 0x4d, 0xf4, 0xb4, 0x8a, 0x78, 0x2d, 0xde, 0xcb, 0x6c, 0x3b, 0x6e, 0x83, 0xd9, 0xa4,
	0xa4, 0x41, 0xfd, 0x15, 0xfe, 0x66, 0x21, 0x89, 0x31, 0x2b, 0xe2, 0x71, 0xf2, 0xbe, 0xf7, 0x66,
	0x5e, 0x20, 0x1f, 0xb1, 0x7b, 0xc5, 0x2d, 0x4d, 0x95, 0x21, 0x41, 0x6a, 0xac, 0x46, 0xad, 0x8c,
	0xaa, 0x3a, 0xdc, 0x91, 0xc6, 0xd2, 0x0e, 0x2c, 0x71, 0xd2, 0xea, 0x2c, 0xb0, 0x03, 0x0a, 0x0f,
	0xf6, 0xf4, 0xc6, 0x3b, 0x72, 0xe0, 0x2a, 0xff, 0x03, 0x70, 0x49, 0x2d, 0xef, 0x3d, 0x73, 0x17,
	0x98, 0x0e, 0x05, 0xdf, 0x68, 0x34, 0x5c, 0xc9, 0x5f, 0xac, 0x34, 0x9a, 0xcb, 0x89, 0x77, 0x6d,
	0x84, 0x78, 0xfb, 0xcd, 0x7f, 0x76, 0xa5, 0x74, 0xcf, 0x25, 0x1a, 0x6a, 0x8d, 0x46, 0x39, 0xbd,
	0x28, 0xbd, 0x8b, 0xbc, 0xf9, 0xe7, 0x01, 0x24, 0x6b, 0xbb, 0x82, 0x5d, 0x40, 0xe2, 0x2e, 0x4f,
	0x21, 0x9b, 0x15, 0x8b, 0x7a, 0x51, 0x0e, 0x28, 0xca, 0x7b, 0xfb, 0xd4, 0x78, 0x89, 0x2d, 0x61,
	0xfe, 0xce, 0x7b, 0x33, 0xa4, 0xcb, 0x6c, 0x56, 0xcc, 0x1b, 0x37, 0xb0, 0x13, 0x48, 0x06, 0xe2,
	0xdb, 0xc1, 0xa4, 0xa7, 0xf6, 0xd9, 0x4f, 0xec, 0x11, 0x20, 0x1c, 0x3e, 0xa5, 0x85, 0x8d, 0xbd,
	0x2a, 0xe3, 0x06, 0x6e, 0xf7, 0xd3, 0x37, 0xb4, 0xfe, 0x91, 0x9a, 0xc8, 0xca, 0x1e, 0x00, 0xe8,
	0x23, 0x04, 0xd5, 0x36, 0xe8, 0x72, 0x3f, 0x28, 0x14, 0x7d, 0xde, 0xeb, 0xd9, 0x44, 0x46, 0x76,
	0x0e, 0x87, 0x5a, 0x09, 0x4a, 0x6f, 0xb3, 0x59, 0x71, 0x5c, 0x1f, 0xd9, 0x82, 0xfe, 0x82, 0xbe,
	0xb1, 0xd2, 0x26, 0xb1, 0xff, 0x72, 0xfd, 0x15, 0x00, 0x00, 0xff, 0xff, 0xd2, 0xa4, 0xae, 0xb1,
	0x05, 0x02, 0x00, 0x00,
}