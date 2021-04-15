package teleopnode

import (
	"context"
	"encoding/json"
	"fmt"
	"log"

	"github.com/golang/protobuf/proto"
	"github.com/gorilla/websocket"
)

func readMessage(ctx context.Context, conn *websocket.Conn) ([]byte, error) {
	type msg struct {
		buf []byte
		err error
	}

	// start the read
	ch := make(chan msg)
	defer close(ch)
	go func() {
		defer close(ch)
		_, buf, err := conn.ReadMessage()

		select {
		case <-ctx.Done():
		case ch <- msg{buf, err}:
		}
	}()

	select {
	case msg := <-ch:
		return msg.buf, msg.err
	case <-ctx.Done():
		return nil, ctx.Err()
	}
}

func readMessages(ctx context.Context, ch chan []byte, conn *websocket.Conn) error {
	defer close(ch)
	defer conn.Close()
	for {
		_, buf, err := conn.ReadMessage()
		if err != nil {
			log.Println("error reading from websocket:", err)
			return err
		}

		if buf == nil {
			log.Println("error: ReadMessage returned nil error and nil buffer")
		}

		select {
		case <-ctx.Done():
			return ctx.Err()
		case ch <- buf:
		}
	}
}

// SendJSON sends a json-encoded message over a websocket
func SendJSON(conn *websocket.Conn, obj interface{}) error {
	buf, err := json.Marshal(obj)
	if err != nil {
		return err
	}

	if err = conn.WriteMessage(websocket.TextMessage, buf); err != nil {
		return err
	}

	// for testing this is implemented to let them know that this concurrent process is done
	// and that the json is now safe to read from the other end
	done()
	return nil
}

// SendJSONPB is like SendJSON but uses the protobuf rules for unmarshalling
// json rather than the regular Go rules.
func SendJSONPB(conn *websocket.Conn, pb proto.Message) error {
	buf, err := MarshalJSONPB(pb)
	if err != nil {
		return err
	}

	if err = conn.WriteMessage(websocket.TextMessage, buf); err != nil {
		return err
	}

	// for testing this is implemented to let them know that this concurrent process is done
	// and that the json is now safe to read from the other end
	done()
	return nil
}

// SendProto sends a protobuf-encoded message over a websocket
func SendProto(conn *websocket.Conn, pb proto.Message) error {
	buf, err := proto.Marshal(pb)
	if err != nil {
		return err
	}

	return conn.WriteMessage(websocket.BinaryMessage, buf)
}

// RecvProto receives a protobuf-encoded message over a websocket
func RecvProto(conn *websocket.Conn, dest proto.Message) error {
	typ, buf, err := conn.ReadMessage()
	if err != nil {
		return err
	}
	if typ != websocket.BinaryMessage {
		return fmt.Errorf("expected frame type to be BinaryMessage but got %d", typ)
	}
	return proto.Unmarshal(buf, dest)
}

// RecvJSON receives a json-encoded message over a websocket
func RecvJSON(conn *websocket.Conn, dest interface{}) error {
	typ, buf, err := conn.ReadMessage()
	if err != nil {
		return err
	}
	if typ != websocket.TextMessage {
		return fmt.Errorf("expected frame type to be TextMessage but got %d", typ)
	}
	return json.Unmarshal(buf, dest)
}

// RecvJSONPB is like RecvJSON but uses the protobuf rules for unmarshalling
// json rather than the regular Go rules.
func RecvJSONPB(conn *websocket.Conn, dest proto.Message) error {
	typ, buf, err := conn.ReadMessage()
	if err != nil {
		return err
	}
	if typ != websocket.TextMessage {
		return fmt.Errorf("expected frame type to be TextMessage but got %d", typ)
	}
	return UnmarshalJSONPB(buf, dest)
}
