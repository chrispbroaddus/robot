// Code generated by protoc-gen-go. DO NOT EDIT.
// source: packages/hal/proto/simulator_stats_response.proto

package hal

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

type SimulatorStatsResponse struct {
	// to subscribe (everying including a protocol, an IPv4 address, and a port number)
	Address string `protobuf:"bytes,1,opt,name=address" json:"address,omitempty"`
	Topic   string `protobuf:"bytes,2,opt,name=topic" json:"topic,omitempty"`
}

func (m *SimulatorStatsResponse) Reset()                    { *m = SimulatorStatsResponse{} }
func (m *SimulatorStatsResponse) String() string            { return proto.CompactTextString(m) }
func (*SimulatorStatsResponse) ProtoMessage()               {}
func (*SimulatorStatsResponse) Descriptor() ([]byte, []int) { return fileDescriptor18, []int{0} }

func (m *SimulatorStatsResponse) GetAddress() string {
	if m != nil {
		return m.Address
	}
	return ""
}

func (m *SimulatorStatsResponse) GetTopic() string {
	if m != nil {
		return m.Topic
	}
	return ""
}

func init() {
	proto.RegisterType((*SimulatorStatsResponse)(nil), "hal.SimulatorStatsResponse")
}

func init() { proto.RegisterFile("packages/hal/proto/simulator_stats_response.proto", fileDescriptor18) }

var fileDescriptor18 = []byte{
	// 129 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xe2, 0x32, 0x2c, 0x48, 0x4c, 0xce,
	0x4e, 0x4c, 0x4f, 0x2d, 0xd6, 0xcf, 0x48, 0xcc, 0xd1, 0x2f, 0x28, 0xca, 0x2f, 0xc9, 0xd7, 0x2f,
	0xce, 0xcc, 0x2d, 0xcd, 0x49, 0x2c, 0xc9, 0x2f, 0x8a, 0x2f, 0x2e, 0x49, 0x2c, 0x29, 0x8e, 0x2f,
	0x4a, 0x2d, 0x2e, 0xc8, 0xcf, 0x2b, 0x4e, 0xd5, 0x03, 0x4b, 0x0b, 0x31, 0x67, 0x24, 0xe6, 0x28,
	0x79, 0x70, 0x89, 0x05, 0xc3, 0x94, 0x05, 0x83, 0x54, 0x05, 0x41, 0x15, 0x09, 0x49, 0x70, 0xb1,
	0x27, 0xa6, 0xa4, 0x14, 0xa5, 0x16, 0x17, 0x4b, 0x30, 0x2a, 0x30, 0x6a, 0x70, 0x06, 0xc1, 0xb8,
	0x42, 0x22, 0x5c, 0xac, 0x25, 0xf9, 0x05, 0x99, 0xc9, 0x12, 0x4c, 0x60, 0x71, 0x08, 0x27, 0x89,
	0x0d, 0x6c, 0xaa, 0x31, 0x20, 0x00, 0x00, 0xff, 0xff, 0x0b, 0xf0, 0xfe, 0xb6, 0x8a, 0x00, 0x00,
	0x00,
}