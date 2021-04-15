package teleopnode

import (
	"context"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"time"

	"github.com/gorilla/mux"
	"github.com/kr/pretty"
	uuid "github.com/satori/go.uuid"
	"github.com/zippyai/zippy/backend/services"
	"github.com/zippyai/zippy/backend/teleop"
	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

// the handler for the operator websocket endpoint
func (a *App) commandVehicle(w http.ResponseWriter, r *http.Request) {
	a.communicateWithVehicle(w, r, control)
}

func (a *App) viewVehicle(w http.ResponseWriter, r *http.Request) {
	a.communicateWithVehicle(w, r, view)
}

func (a *App) communicateWithVehicle(w http.ResponseWriter, r *http.Request, listenerType listenerType) {
	vars := mux.Vars(r)
	sessionID := vars["sid"]
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	if !services.VehicleActive(a.storage, sessionID) {
		err := fmt.Errorf("vehicle session with id: %s is not active or doesn't exist", sessionID)
		log.Println(err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
	}

	requesterID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer func(now time.Time) {
		websocketLifeSpan.With("endpoint", pathTmpl, "requester", requesterID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	log.Println("at handleSubscribeRaw:", sessionID)
	defer log.Println("returning from handleSubscribeRaw")

	// get the interval query parameter
	interval, err := durationOrZero(r.URL.Query().Get("interval"))
	if err != nil {
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	// upgrade from ordinary http request to websocket
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("error upgrading to websocket connection:", err)
		return
	}
	openWebsockets.With("endpoint", pathTmpl, "requester", requesterID).Add(1)
	defer openWebsockets.With("endpoint", pathTmpl, "requester", requesterID).Add(-1)

	// get the relay or create it
	relay := a.relayOrCreate(sessionID)

	// create cancellation context
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	// register listener
	listener := relay.listen(listenerType, cancel)
	defer relay.unlisten(listener.id)

	// read from the connection
	fromOperator := make(chan []byte)
	go readMessages(ctx, fromOperator, conn)

	// for testing this is implemented to let them know that this concurrent process is done
	// and that the websocket conn setup is now done
	done()

	// loop until the client hangs up
	var prev time.Time
OuterLoop:
	for {
		select {
		case <-ctx.Done():
			log.Println("context is done, breaking out of handleSubscribeRaw")
			break OuterLoop

		case buf := <-fromOperator:
			if buf == nil {
				log.Println("channel closed, breaking out of handleSubscribeRaw")
				return
			}
			websocketFramesSent.With("endpoint", pathTmpl, "requester", requesterID).Add(1)

			// parse message
			var msg teleopproto.BackendMessage
			err = UnmarshalJSONPB(buf, &msg)
			if err != nil {
				log.Println("error parsing command from operator:", err)
				continue
			}

			// if there is not a commandID set by the frontend set it now
			if msg.Id == "" {
				msg.Id = uuid.NewV4().String()
			}

			// record the time we got the message to calc total time when we write
			timingManager.Start(msg.Id)

			log.Printf("message we got from operator message: %T\n", msg.GetPayload())

			// send to vehicle
			relay.processCommand(&msg)

		case msg := <-listener.ch:
			if time.Since(prev) < interval {
				log.Println("dropping a frame due to interval")
				continue
			}
			prev = time.Now()

			// send message out
			err = SendJSONPB(conn, msg)
			if err != nil {
				log.Println("error writing to operator:", err)
				return
			}
		}
	}
}

func (a *App) handleSubscribeLocation(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	sessionID := vars["sid"]
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	if !services.VehicleActive(a.storage, sessionID) {
		err := fmt.Errorf("vehicle session with id: %s is not active or doesn't exist", sessionID)
		log.Println(err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
	}

	requesterID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer func(now time.Time) {
		websocketLifeSpan.With("endpoint", pathTmpl, "requester", sessionID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	log.Println("at handleSubscribeLocation:", sessionID)
	defer log.Println("returning from handleSubscribeLocation")

	// upgrade from ordinary http request to websocket
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("error upgrading to websocket connection:", err)
		return
	}
	defer conn.Close()
	openWebsockets.With("endpoint", pathTmpl, "requester", requesterID).Add(1)
	defer openWebsockets.With("endpoint", pathTmpl, "requester", requesterID).Add(-1)

	// get the relay or create it
	relay := a.relayOrCreate(sessionID)

	// create cancellation context
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	// register listener
	listener := relay.listen(location, cancel)
	defer relay.unlisten(listener.id)

	// read from the connection
	fromOperator := make(chan []byte)
	go readMessages(ctx, fromOperator, conn)

	// for testing this is implemented to let them know that this concurrent process is done
	// and that the websocket conn setup is now done
	done()

	// loop until the client hangs up
OuterLoop:
	for {
		select {
		case <-ctx.Done():
			log.Println("context is done, breaking out of handleSubscribeLocation")
			break OuterLoop

		case buf := <-fromOperator:
			if buf == nil {
				log.Println("channel closed, breaking out of handleSubscribeRaw")
				return
			}

		case msg := <-listener.ch:
			pl, ok := msg.GetPayload().(*teleopproto.VehicleMessage_Gps)
			if !ok {
				continue
			}

			websocketFramesSent.With("endpoint", pathTmpl, "requester", requesterID).Add(1)
			err := SendJSON(conn, teleop.LocationSampleFromProto(pl.Gps))
			if err != nil {
				log.Println("error writing location to subscriber:", err)
				return
			}
		}
	}
}

func (a *App) handleSignalWebrtc(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	sessionID := vars["sid"]
	clientID := vars["clid"]
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	if !services.VehicleActive(a.storage, sessionID) {
		err := fmt.Errorf("vehicle session with id: %s is not active or doesn't exist", sessionID)
		log.Println(err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
	}

	requesterID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer func(now time.Time) {
		websocketLifeSpan.With("endpoint", pathTmpl, "requester", sessionID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	log.Println("at handleSignalWebrtc: ", sessionID)
	defer log.Println("returning from handleSignalWebrtc")

	// upgrade from ordinary http request to websocket
	clientConn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("error upgrading to websocket connection:", err)
		return
	}
	openWebsockets.With("endpoint", pathTmpl, "requester", requesterID).Add(1)
	defer openWebsockets.With("endpoint", pathTmpl, "requester", requesterID).Add(-1)

	// get the relay or create it
	relay := a.relayOrCreate(sessionID)
	if err := services.UpdateWebrtcStatus(a.storage, sessionID, clientID, teleopproto.RTCStatus_Disconnected); err != nil {
		log.Printf("unable to update rtc state of the between user: %s and vehicle: %s error: %s\n", clientID, sessionID, err)
	}

	// create cancellation context
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	// read from the connection
	fromOperator := make(chan []byte)
	go readMessages(ctx, fromOperator, clientConn)

	// register listener
	listener := relay.listen(webrtc, cancel)
	defer relay.unlisten(listener.id)

	// for testing this is implemented to let them know that this concurrent process is done
	// and that the websocket conn setup is now done
	done()

	// loop until the client hangs up
OuterLoop:
	for {
		select {
		case <-ctx.Done():
			log.Println("context is done, breaking out of handleSubscribeCamera")
			break OuterLoop

		case blob := <-fromOperator:
			if blob == nil {
				log.Println("channel closed, breaking out of handleSignalWebrtc response empty")
				return
			}

			websocketFramesSent.With("endpoint", pathTmpl, "requester", requesterID).Add(1)
			msg := new(teleop.SDPMessage)
			if err := json.Unmarshal(blob, &msg); err != nil {
				log.Println("error parsing sdp offer from operator: ", err)
				continue
			}

			log.Printf("got SDPmsg from opporator: %+v\n", msg)

			var protoMsg *teleopproto.BackendMessage
			// handle the different messages that can come across this connection
			if msg.VideoRequest != nil {
				log.Println("VideoRequest recieved from client")
				protoMsg = teleop.NewProtoVideoRequest(msg.VideoRequest)
			} else if msg.Request != nil {
				// msg.Request either sdp offer or answer
				log.Println("SDPRequest received from client")
				if err := services.UpdateSDPStatus(a.storage, sessionID, clientID, msg.Request.Status); err != nil {
					log.Printf("unable to update sdp state of the between user: %s and vehicle: %s error: %s\n", clientID, sessionID, err)
				}
				protoMsg = teleop.NewProtoSDPRequest(msg.Request)
			} else if msg.Confirmation != nil {
				// msg.Confirmation client confirming they are connected after offer/answer
				log.Println("SDPConfirmation recieved from client")
				newStatus := teleopproto.RTCStatus_Disconnected
				if msg.Confirmation.Connected {
					newStatus = teleopproto.RTCStatus_Connected
					err = services.AddVehicleViewer(a.storage, sessionID, clientID)
					if err != nil {
						log.Println("error setting user as viewing vehicle: ", err)
					}
				}

				if err := services.UpdateWebrtcStatus(a.storage, sessionID, clientID, newStatus); err != nil {
					log.Printf("unable to update rtc state of the between user: %s and vehicle: %s error: %s\n", clientID, sessionID, err)
				}
				protoMsg = teleop.NewProtoConfirmation(msg.Confirmation)
			} else if msg.ICE != nil {
				log.Println("ICECandidate recieved from client")
				protoMsg = teleop.NewProtoICECandidate(msg.ICE)
			} else {
				log.Println("raw message we got: ", string(blob))
				log.Println("unhandled message type")
				continue
			}

			log.Println("got SDPMessage from operator...")
			pretty.Log(protoMsg)

			// send to the vehicle
			relay.processCommand(protoMsg)

		case msg := <-listener.ch:
			// payload to send to client from vehicle
			var sendMsg *teleop.SDPMessage

			// handle sdp request messages containing either offer or answer
			sdpRequest := msg.GetSdpRequest()
			if sdpRequest != nil {
				sendMsg = &teleop.SDPMessage{
					Request: teleop.SDPRequestFromProto(sdpRequest),
				}

				if err := services.UpdateSDPStatus(a.storage, sessionID, clientID, sdpRequest.Status); err != nil {
					log.Printf("unable to update sdp state of the between user: %s and vehicle: %s error: %s\n", clientID, sessionID, err)
				}
			}

			// handle sdp confirmation messages containing true/false as to connection state
			sdpConfirmation := msg.GetSdpComfirmation()
			if sdpConfirmation != nil {
				sendMsg = &teleop.SDPMessage{
					Confirmation: teleop.SDPConfirmationFromProto(sdpConfirmation),
				}

				newStatus := teleopproto.RTCStatus_Disconnected
				if sdpConfirmation.Connected {
					newStatus = teleopproto.RTCStatus_Connected
					err = services.AddVehicleViewer(a.storage, sessionID, clientID)
					if err != nil {
						log.Println("error setting user as viewing vehicle: ", err)
					}
				}

				if err := services.UpdateWebrtcStatus(a.storage, sessionID, clientID, newStatus); err != nil {
					log.Printf("unable to update rtc state of the between user: %s and vehicle: %s error: %s\n", clientID, sessionID, err)
				}
			}

			// routes newly found ice candidates to the client
			iceCandidate := msg.GetIceCandidate()
			if iceCandidate != nil {
				sendMsg = &teleop.SDPMessage{
					ICE: teleop.ICECandidateFromProto(iceCandidate),
				}
			}

			if sendMsg != nil {
				log.Println("webrtc connection loop got message from vehicle:")
				pretty.Log(msg)

				log.Println("sending to operator:")
				pretty.Log(sendMsg)
			}

			// send message out to the client
			err := SendJSON(clientConn, sendMsg)
			if err != nil {
				log.Println("error sending sdp request to client:", err)
				return
			}
		}
	}
}
