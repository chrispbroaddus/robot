package main

import (
	"fmt"
	"os"

	arg "github.com/alexflint/go-arg"
	"github.com/gorilla/websocket"
	"github.com/zippyai/zippy/backend/teleopnode"
	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

func main() {
	var args struct {
		Host      string
		ImagePath string
		VehicleID string
	}
	args.VehicleID = "r01"
	arg.MustParse(&args)

	url := fmt.Sprintf("ws://%s/api/teleop/%s/operator", args.Host, args.VehicleID)
	fmt.Println(url)

	var d websocket.Dialer
	conn, _, err := d.Dial(url, nil)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	// read updates
	fmt.Println("waiting for udpates...")
	var msg teleopproto.VehicleMessage
	for {
		err := teleopnode.RecvJSONPB(conn, &msg)
		if err != nil {
			fmt.Println(err)
			os.Exit(1)
		}

		switch p := msg.Payload.(type) {
		case *teleopproto.VehicleMessage_Frame:
			fmt.Printf("received frame of size %dx%d\n", p.Frame.GetWidth(), p.Frame.GetHeight())
		case *teleopproto.VehicleMessage_Gps:
			fmt.Printf("received gps fix at %f, %f\n", p.Gps.GetLatitude(), p.Gps.GetLongitude())
		default:
			fmt.Printf("received message of type %T\n", p)
		}
	}
}
