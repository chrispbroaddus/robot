// Code generated by protoc-gen-go. DO NOT EDIT.
// source: packages/hal/proto/vcu_ik_control_command.proto

package hal

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"
import core "github.com/zippyai/zippy/packages/core/proto"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

type VCUIKControlSegment struct {
	TargetStartTime *core.SystemTimestamp `protobuf:"bytes,1,opt,name=target_start_time,json=targetStartTime" json:"target_start_time,omitempty"`
	// / Pitch-rotation on the rails:
	// /     + : counter-clockwise, when seeing the vehicle from right side
	// /     - : clockwise, when seeing the vehicle from right side
	// / Units: radian
	LeftRailPitch  float32 `protobuf:"fixed32,2,opt,name=left_rail_pitch,json=leftRailPitch" json:"left_rail_pitch,omitempty"`
	RightRailPitch float32 `protobuf:"fixed32,3,opt,name=right_rail_pitch,json=rightRailPitch" json:"right_rail_pitch,omitempty"`
	// / The height of the joint in z-axis
	// / Units: meter
	LeftRailElevation  float32 `protobuf:"fixed32,4,opt,name=left_rail_elevation,json=leftRailElevation" json:"left_rail_elevation,omitempty"`
	RightRailElevation float32 `protobuf:"fixed32,5,opt,name=right_rail_elevation,json=rightRailElevation" json:"right_rail_elevation,omitempty"`
	// / Linear velocity along the curve; may be negative.
	// / Units: m/s
	LinearVelocityMetersPerSecond float32 `protobuf:"fixed32,6,opt,name=linearVelocityMetersPerSecond" json:"linearVelocityMetersPerSecond,omitempty"`
	// / Curvature (inverse radius)
	// / Units: m^{-1}
	CurvatureInverseMeters float32 `protobuf:"fixed32,7,opt,name=curvatureInverseMeters" json:"curvatureInverseMeters,omitempty"`
}

func (m *VCUIKControlSegment) Reset()                    { *m = VCUIKControlSegment{} }
func (m *VCUIKControlSegment) String() string            { return proto.CompactTextString(m) }
func (*VCUIKControlSegment) ProtoMessage()               {}
func (*VCUIKControlSegment) Descriptor() ([]byte, []int) { return fileDescriptor24, []int{0} }

func (m *VCUIKControlSegment) GetTargetStartTime() *core.SystemTimestamp {
	if m != nil {
		return m.TargetStartTime
	}
	return nil
}

func (m *VCUIKControlSegment) GetLeftRailPitch() float32 {
	if m != nil {
		return m.LeftRailPitch
	}
	return 0
}

func (m *VCUIKControlSegment) GetRightRailPitch() float32 {
	if m != nil {
		return m.RightRailPitch
	}
	return 0
}

func (m *VCUIKControlSegment) GetLeftRailElevation() float32 {
	if m != nil {
		return m.LeftRailElevation
	}
	return 0
}

func (m *VCUIKControlSegment) GetRightRailElevation() float32 {
	if m != nil {
		return m.RightRailElevation
	}
	return 0
}

func (m *VCUIKControlSegment) GetLinearVelocityMetersPerSecond() float32 {
	if m != nil {
		return m.LinearVelocityMetersPerSecond
	}
	return 0
}

func (m *VCUIKControlSegment) GetCurvatureInverseMeters() float32 {
	if m != nil {
		return m.CurvatureInverseMeters
	}
	return 0
}

type VCUIKControlCommand struct {
	// / Restricted in options file to max_count: 31
	Segments []*VCUIKControlSegment `protobuf:"bytes,1,rep,name=segments" json:"segments,omitempty"`
}

func (m *VCUIKControlCommand) Reset()                    { *m = VCUIKControlCommand{} }
func (m *VCUIKControlCommand) String() string            { return proto.CompactTextString(m) }
func (*VCUIKControlCommand) ProtoMessage()               {}
func (*VCUIKControlCommand) Descriptor() ([]byte, []int) { return fileDescriptor24, []int{1} }

func (m *VCUIKControlCommand) GetSegments() []*VCUIKControlSegment {
	if m != nil {
		return m.Segments
	}
	return nil
}

func init() {
	proto.RegisterType((*VCUIKControlSegment)(nil), "hal.VCUIKControlSegment")
	proto.RegisterType((*VCUIKControlCommand)(nil), "hal.VCUIKControlCommand")
}

func init() { proto.RegisterFile("packages/hal/proto/vcu_ik_control_command.proto", fileDescriptor24) }

var fileDescriptor24 = []byte{
	// 336 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0x7c, 0x91, 0xc1, 0x4b, 0xe3, 0x40,
	0x14, 0x87, 0x49, 0xb3, 0xdb, 0x5d, 0xa6, 0xec, 0xd6, 0x4e, 0x55, 0x82, 0x20, 0x94, 0x0a, 0x92,
	0x53, 0x22, 0x55, 0xbc, 0x4b, 0xf5, 0x50, 0x8a, 0x50, 0x12, 0xed, 0x35, 0x8c, 0xd3, 0x67, 0x32,
	0x74, 0x92, 0x09, 0x33, 0xaf, 0x81, 0xfe, 0x0b, 0xfe, 0xd5, 0x92, 0x89, 0x6d, 0x2c, 0xa8, 0xd7,
	0xf7, 0xbe, 0xef, 0xbb, 0xfc, 0x48, 0x58, 0x32, 0xbe, 0x66, 0x29, 0x98, 0x30, 0x63, 0x32, 0x2c,
	0xb5, 0x42, 0x15, 0x56, 0x7c, 0x93, 0x88, 0x75, 0xc2, 0x55, 0x81, 0x5a, 0xc9, 0x84, 0xab, 0x3c,
	0x67, 0xc5, 0x2a, 0xb0, 0x4f, 0xea, 0x66, 0x4c, 0x9e, 0x5d, 0xec, 0x2d, 0xae, 0x34, 0x7c, 0x68,
	0x28, 0x72, 0x30, 0xc8, 0xf2, 0xb2, 0x21, 0xc7, 0x6f, 0x2e, 0x19, 0x2e, 0xa7, 0xcf, 0xb3, 0xf9,
	0xb4, 0x09, 0xc5, 0x90, 0xe6, 0x50, 0x20, 0xbd, 0x23, 0x03, 0x64, 0x3a, 0x05, 0x4c, 0x0c, 0x32,
	0x8d, 0x49, 0xed, 0x79, 0xce, 0xc8, 0xf1, 0x7b, 0x93, 0x93, 0xa0, 0xee, 0x05, 0xf1, 0xd6, 0x20,
	0xe4, 0x4f, 0xbb, 0x5e, 0xd4, 0x6f, 0xf8, 0xb8, 0xc6, 0xeb, 0x2b, 0xbd, 0x24, 0x7d, 0x09, 0xaf,
	0x98, 0x68, 0x26, 0x64, 0x52, 0x0a, 0xe4, 0x99, 0xd7, 0x19, 0x39, 0x7e, 0x27, 0xfa, 0x57, 0x9f,
	0x23, 0x26, 0xe4, 0xa2, 0x3e, 0x52, 0x9f, 0x1c, 0x69, 0x91, 0x66, 0x07, 0xa0, 0x6b, 0xc1, 0xff,
	0xf6, 0xde, 0x92, 0x01, 0x19, 0xb6, 0x45, 0x90, 0x50, 0x31, 0x14, 0xaa, 0xf0, 0x7e, 0x59, 0x78,
	0xb0, 0xab, 0x3e, 0xec, 0x1e, 0xf4, 0x8a, 0x1c, 0x7f, 0x2a, 0xb7, 0xc2, 0x6f, 0x2b, 0xd0, 0x7d,
	0xbd, 0x35, 0xee, 0xc9, 0xb9, 0x14, 0x05, 0x30, 0xbd, 0x04, 0xa9, 0xb8, 0xc0, 0xed, 0x23, 0x20,
	0x68, 0xb3, 0x00, 0x1d, 0x03, 0x57, 0xc5, 0xca, 0xeb, 0x5a, 0xf5, 0x67, 0x88, 0xde, 0x92, 0x53,
	0xbe, 0xd1, 0x15, 0xc3, 0x8d, 0x86, 0x59, 0x51, 0x81, 0x36, 0xd0, 0x20, 0xde, 0x1f, 0xab, 0x7f,
	0xf3, 0x1d, 0xcf, 0x0f, 0xb7, 0x98, 0x36, 0x9b, 0xd2, 0x1b, 0xf2, 0xd7, 0x34, 0xb3, 0x18, 0xcf,
	0x19, 0xb9, 0x7e, 0x6f, 0xe2, 0x05, 0x19, 0x93, 0xc1, 0x17, 0xbb, 0x45, 0x7b, 0xf2, 0xa5, 0x6b,
	0x07, 0xbe, 0x7e, 0x0f, 0x00, 0x00, 0xff, 0xff, 0xb0, 0x2a, 0x34, 0x1c, 0x3d, 0x02, 0x00, 0x00,
}
