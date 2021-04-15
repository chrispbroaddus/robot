package teleopnode

import (
	"bytes"

	"github.com/golang/protobuf/jsonpb"
	"github.com/golang/protobuf/proto"
)

var jsonMarshaler jsonpb.Marshaler
var jsonUnmarshaler jsonpb.Unmarshaler

// MarshalJSONPB uses the protobuf rules for marshalling json rather than the
// regular Go rules.
func MarshalJSONPB(pb proto.Message) ([]byte, error) {
	var b bytes.Buffer
	err := jsonMarshaler.Marshal(&b, pb)
	if err != nil {
		return nil, err
	}
	return b.Bytes(), nil
}

// UnmarshalJSONPB uses the protobuf rules for unmarshalling json rather than
// the regular Go rules.
func UnmarshalJSONPB(buf []byte, pb proto.Message) error {
	return jsonUnmarshaler.Unmarshal(bytes.NewReader(buf), pb)
}
