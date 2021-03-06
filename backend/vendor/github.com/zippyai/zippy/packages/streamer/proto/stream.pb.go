// Code generated by protoc-gen-go. DO NOT EDIT.
// source: packages/streamer/proto/stream.proto

package streamer

import proto "github.com/golang/protobuf/proto"
import fmt "fmt"
import math "math"

// Reference imports to suppress errors if they are not otherwise used.
var _ = proto.Marshal
var _ = fmt.Errorf
var _ = math.Inf

// / Stream represents options for a ZmqVideoCapturer
type Stream struct {
	// / Address of ZMQ socket to which we should subscribe
	Address string `protobuf:"bytes,1,opt,name=address" json:"address,omitempty"`
	// / ZMQ topic to which we should subscribe
	Topic string `protobuf:"bytes,2,opt,name=topic" json:"topic,omitempty"`
	// / Width of image streamed over the network (does not have to match input image dimensions)
	OutputWidth int32 `protobuf:"varint,3,opt,name=output_width,json=outputWidth" json:"output_width,omitempty"`
	// / Height of image streamed over the network (does not have to match input image dimensions)
	OutputHeight int32 `protobuf:"varint,4,opt,name=output_height,json=outputHeight" json:"output_height,omitempty"`
}

func (m *Stream) Reset()                    { *m = Stream{} }
func (m *Stream) String() string            { return proto.CompactTextString(m) }
func (*Stream) ProtoMessage()               {}
func (*Stream) Descriptor() ([]byte, []int) { return fileDescriptor1, []int{0} }

func (m *Stream) GetAddress() string {
	if m != nil {
		return m.Address
	}
	return ""
}

func (m *Stream) GetTopic() string {
	if m != nil {
		return m.Topic
	}
	return ""
}

func (m *Stream) GetOutputWidth() int32 {
	if m != nil {
		return m.OutputWidth
	}
	return 0
}

func (m *Stream) GetOutputHeight() int32 {
	if m != nil {
		return m.OutputHeight
	}
	return 0
}

func init() {
	proto.RegisterType((*Stream)(nil), "streamer.Stream")
}

func init() { proto.RegisterFile("packages/streamer/proto/stream.proto", fileDescriptor1) }

var fileDescriptor1 = []byte{
	// 154 bytes of a gzipped FileDescriptorProto
	0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff, 0xe2, 0x52, 0x29, 0x48, 0x4c, 0xce,
	0x4e, 0x4c, 0x4f, 0x2d, 0xd6, 0x2f, 0x2e, 0x29, 0x4a, 0x4d, 0xcc, 0x4d, 0x2d, 0xd2, 0x2f, 0x28,
	0xca, 0x2f, 0xc9, 0x87, 0x72, 0xf5, 0xc0, 0x1c, 0x21, 0x0e, 0x98, 0xa4, 0x52, 0x03, 0x23, 0x17,
	0x5b, 0x30, 0x98, 0x23, 0x24, 0xc1, 0xc5, 0x9e, 0x98, 0x92, 0x52, 0x94, 0x5a, 0x5c, 0x2c, 0xc1,
	0xa8, 0xc0, 0xa8, 0xc1, 0x19, 0x04, 0xe3, 0x0a, 0x89, 0x70, 0xb1, 0x96, 0xe4, 0x17, 0x64, 0x26,
	0x4b, 0x30, 0x81, 0xc5, 0x21, 0x1c, 0x21, 0x45, 0x2e, 0x9e, 0xfc, 0xd2, 0x92, 0x82, 0xd2, 0x92,
	0xf8, 0xf2, 0xcc, 0x94, 0x92, 0x0c, 0x09, 0x66, 0x05, 0x46, 0x0d, 0xd6, 0x20, 0x6e, 0x88, 0x58,
	0x38, 0x48, 0x48, 0x48, 0x99, 0x8b, 0x17, 0xaa, 0x24, 0x23, 0x35, 0x33, 0x3d, 0xa3, 0x44, 0x82,
	0x05, 0xac, 0x06, 0xaa, 0xcf, 0x03, 0x2c, 0x96, 0xc4, 0x06, 0x76, 0x93, 0x31, 0x20, 0x00, 0x00,
	0xff, 0xff, 0x06, 0x5e, 0x6b, 0x5d, 0xbb, 0x00, 0x00, 0x00,
}
