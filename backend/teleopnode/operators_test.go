package teleopnode

import (
	"fmt"
	"net/http"
	"testing"

	uuid "github.com/satori/go.uuid"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"github.com/zippyai/zippy/backend/teleop"
	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

func TestApp_SubscribeLocation(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()

	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())
	vehicleConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID), sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	subscribeConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/location/subscribe", sessionCookie.Value), authHeader)
	require.NoError(t, err)
	defer subscribeConn.Close()
	<-doneChan

	SendProto(vehicleConn, makeGPS(1.1, 2.2))
	<-doneChan
	<-doneChan

	var s teleop.LocationSample
	err = RecvJSON(subscribeConn, &s)
	require.NoError(t, err)
	assert.Equal(t, 1.1, s.Location.Lat)
	assert.Equal(t, 2.2, s.Location.Lon)

	SendProto(vehicleConn, makeGPS(3.3, 4.4))
	<-doneChan
	<-doneChan

	err = RecvJSON(subscribeConn, &s)
	require.NoError(t, err)
	assert.Equal(t, 3.3, s.Location.Lat)
	assert.Equal(t, 4.4, s.Location.Lon)
}

func TestApp_SubscribeCamera(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()

	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())
	vehicleConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID), sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	subscribeConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/camera/all/subscribe", sessionCookie.Value), authHeader)
	require.NoError(t, err)
	defer subscribeConn.Close()
	<-doneChan

	SendProto(vehicleConn, makeFrame("frame1", "forward"))
	<-doneChan
	<-doneChan

	var s teleop.CameraSample
	err = RecvJSON(subscribeConn, &s)
	require.NoError(t, err)
	assert.Equal(t, "frame1", tag(s.Image.Content))
	assert.Equal(t, "forward", s.Camera)

	SendProto(vehicleConn, makeFrame("frame2", "backward"))
	<-doneChan
	<-doneChan

	err = RecvJSON(subscribeConn, &s)
	require.NoError(t, err)
	assert.Equal(t, "frame2", tag(s.Image.Content))
	assert.Equal(t, "backward", s.Camera)
}

func TestApp_ClientSendSDPOfferAnswer(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()

	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())
	url := fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID)
	vehicleConn, err := dialWS(wsServer.URL, url, sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	url = fmt.Sprintf("/api/v1/ws/vehicle/%s/user/%s/subscribe", sessionCookie.Value, clientID)
	webrtcConn, err := dialWS(wsServer.URL, url, authHeader)
	require.NoError(t, err)
	defer webrtcConn.Close()
	<-doneChan

	// Send the initial sdp offer to the vehicle
	clientSDPOffer := newSPDReq("sdp", teleopproto.SDPStatus_Offered)
	err = tSendJSON(webrtcConn, teleop.SDPMessage{Request: clientSDPOffer})
	require.NoError(t, err)
	<-doneChan

	// read the offer that was sent
	var vehicleOffer teleopproto.BackendMessage
	err = RecvProto(vehicleConn, &vehicleOffer)
	require.NoError(t, err)
	receivedOffer := vehicleOffer.GetSdpRequest()
	assert.NotNil(t, receivedOffer)
	assert.Equal(t, clientSDPOffer.SDP, receivedOffer.Sdp)

	clientSDPAnswer := newSPDReq("sdp", teleopproto.SDPStatus_Answered)
	err = tSendJSON(webrtcConn, teleop.SDPMessage{Request: clientSDPAnswer})
	require.NoError(t, err)
	<-doneChan

	var vehicleAnswer teleopproto.BackendMessage
	err = RecvProto(vehicleConn, &vehicleAnswer)
	require.NoError(t, err)
	receivedAnswer := vehicleAnswer.GetSdpRequest()
	assert.NotNil(t, receivedOffer)
	assert.Equal(t, clientSDPAnswer.SDP, receivedAnswer.Sdp)
}

func TestApp_VehicleSendSDPOfferAnswer(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()

	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())

	url := fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID)
	vehicleConn, err := dialWS(wsServer.URL, url, sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	url = fmt.Sprintf("/api/v1/ws/vehicle/%s/user/%s/subscribe", sessionCookie.Value, clientID)
	clientConn, err := dialWS(wsServer.URL, url, authHeader)
	require.NoError(t, err)
	defer clientConn.Close()
	<-doneChan

	// Send the initial sdp offer to the vehicle
	vehicleSDPOffer := newpbSPDReq("sdp", teleopproto.SDPStatus_Offered)
	err = SendProto(vehicleConn, vehicleSDPOffer)
	require.NoError(t, err)
	<-doneChan
	<-doneChan

	// read the offer that was sent
	vehicleOffer := new(teleop.SDPMessage)
	err = RecvJSON(clientConn, &vehicleOffer)
	require.NoError(t, err)
	assert.Equal(t, vehicleSDPOffer.GetSdpRequest().Sdp, vehicleOffer.Request.SDP)

	vehicleSDPAnswer := newpbSPDReq("sdp", teleopproto.SDPStatus_Answered)
	err = SendProto(vehicleConn, vehicleSDPAnswer)
	require.NoError(t, err)
	<-doneChan
	<-doneChan

	clientAnswer := new(teleop.SDPMessage)
	err = RecvJSON(clientConn, &clientAnswer)
	assert.Equal(t, vehicleSDPAnswer.GetSdpRequest().Sdp, clientAnswer.Request.SDP)
}

func newSPDReq(sdp string, status teleopproto.SDPStatus) *teleop.SDPRequest {
	return &teleop.SDPRequest{
		SDP:    sdp,
		Status: status,
	}
}

func newpbSPDReq(sdp string, status teleopproto.SDPStatus) *teleopproto.VehicleMessage {
	req := &teleopproto.SDPRequest{
		Sdp:    sdp,
		Status: status,
	}

	return teleop.NewSDPRequestMessage(req)
}

func TestApp_ICECandidates(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()

	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())
	url := fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID)
	vehicleConn, err := dialWS(wsServer.URL, url, sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	url = fmt.Sprintf("/api/v1/ws/vehicle/%s/user/%s/subscribe", sessionCookie.Value, clientID)
	clientConn, err := dialWS(wsServer.URL, url, authHeader)
	require.NoError(t, err)
	defer clientConn.Close()
	<-doneChan

	clientIceReq := newICEReq("candidate", "audio", 0)
	err = tSendJSON(clientConn, teleop.SDPMessage{ICE: clientIceReq})
	require.NoError(t, err)
	<-doneChan

	var vehicleICE teleopproto.BackendMessage
	err = RecvProto(vehicleConn, &vehicleICE)
	require.NoError(t, err)
	receivedICE := vehicleICE.GetIceCandidate()
	assert.NotNil(t, receivedICE)
	assert.Equal(t, clientIceReq.Candidate, receivedICE.Candidate)
	assert.Equal(t, clientIceReq.SDPMid, receivedICE.SdpMid)
	assert.Equal(t, clientIceReq.SDPMLineIndex, int(receivedICE.SdpMlineIndex))

	vehicleMsg := newpbICEReq("candidate2", "video", 1)
	err = SendProto(vehicleConn, vehicleMsg)
	assert.NoError(t, err)
	<-doneChan
	<-doneChan

	vehicleICEReq := vehicleMsg.GetIceCandidate()
	assert.NotNil(t, vehicleICEReq)

	clientIce := new(teleop.SDPMessage)
	err = RecvJSON(clientConn, clientIce)
	assert.Equal(t, vehicleICEReq.Candidate, clientIce.ICE.Candidate)
	assert.Equal(t, vehicleICEReq.SdpMid, clientIce.ICE.SDPMid)
	assert.Equal(t, int(vehicleICEReq.SdpMlineIndex), clientIce.ICE.SDPMLineIndex)
}

func TestApp_SDPConfirmation(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()

	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())

	url := fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID)
	vehicleConn, err := dialWS(wsServer.URL, url, sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	url = fmt.Sprintf("/api/v1/ws/vehicle/%s/user/%s/subscribe", sessionCookie.Value, clientID)
	clientConn, err := dialWS(wsServer.URL, url, authHeader)
	require.NoError(t, err)
	defer clientConn.Close()
	<-doneChan

	//	ensure that after Sending a connected message that we're connected
	err = tSendJSON(clientConn, teleop.SDPMessage{Confirmation: &teleop.SDPConfirmation{Connected: true}})
	assert.NoError(t, err)
	<-doneChan

	// just wait for response so no race condition
	var msg teleopproto.VehicleMessage
	RecvProto(vehicleConn, &msg)

	//	ensure that after Sending a dis-connected message that we're dis-connected
	err = tSendJSON(clientConn, teleop.SDPMessage{Confirmation: &teleop.SDPConfirmation{Connected: false}})
	assert.NoError(t, err)
	<-doneChan

	// just wait for response so no race condition
	RecvProto(vehicleConn, &msg)
}

func newICEReq(candidate, sdpMid string, sdpMLineIndex int) *teleop.ICECandidate {
	return &teleop.ICECandidate{
		Candidate:     candidate,
		SDPMid:        sdpMid,
		SDPMLineIndex: sdpMLineIndex,
	}
}

func newpbICEReq(candidate, sdpMid string, sdpMLineIndex int) *teleopproto.VehicleMessage {
	req := &teleopproto.ICECandidate{
		Candidate:     candidate,
		SdpMid:        sdpMid,
		SdpMlineIndex: int32(sdpMLineIndex),
	}

	return teleop.NewICECandidateMessage(req)
}
