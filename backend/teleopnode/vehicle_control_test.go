package teleopnode

import (
	"encoding/json"
	"fmt"
	"net/http"
	"testing"

	"github.com/gorilla/websocket"
	uuid "github.com/satori/go.uuid"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"github.com/zippyai/zippy/backend/teleop"
	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

func TestVehicleControl_RequestControlFailure(t *testing.T) {
	// request failed due to bad vehicleID
	requestControlURL := fmt.Sprintf("%s/vehicle/%s/request-control", baseAPIURI, "bad_vehicle_id")
	req, err := makeReq(server.URL, requestControlURL, "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusNotFound, resp.StatusCode)

	// request failed due to no auth
	req, err = makeReq(server.URL, requestControlURL, "GET", nil, badCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusUnauthorized, resp.StatusCode)
}

func TestVehicleControl_RelinquishControlFailure(t *testing.T) {
	// request failed due to bad vehicleID
	requestControlURL := fmt.Sprintf("%s/vehicle/%s/relinquish-control", baseAPIURI, "bad_vehicle_id")
	req, err := makeReq(server.URL, requestControlURL, "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusNotFound, resp.StatusCode)

	// request failed due to no auth
	req, err = makeReq(server.URL, requestControlURL, "GET", nil, badCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusUnauthorized, resp.StatusCode)
}

func TestVehicleControl_AuthSessionFailure(t *testing.T) {
	sessionAuthURL := fmt.Sprintf("%s/vehicle/auth", baseAPIURI)

	// request failed due to malformed json body
	req, err := makeReq(server.URL, sessionAuthURL, "POST", "{:}", adminCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusBadRequest, resp.StatusCode)

	authBody := &teleop.VehicleAuthView{
		Token:     "bad_token",
		VehicleID: "bad_id",
	}

	// request failed due to bad auth information
	req, err = makeReq(server.URL, sessionAuthURL, "POST", authBody, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusUnauthorized, resp.StatusCode)

	// request failed due to not being authed
	req, err = makeReq(server.URL, sessionAuthURL, "POST", authBody, badCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusUnauthorized, resp.StatusCode)
}

func TestVehicleControl_HandoffControl(t *testing.T) {
	newUserCookie := loginNewUser(server.URL, "bob", "builder", t)
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	vehicleID := uuid.NewV4().String()
	sessionCookie, err := newSessionCookie(vehicleID)
	require.NoError(t, err)
	sessionHeader := http.Header{}
	sessionHeader.Add("Cookie", sessionCookie.String())
	registerURL := fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleID)
	vehicleConn, err := dialWS(wsServer.URL, registerURL, sessionHeader)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	connectToVehicle(sessionCookie.Value, "admin", adminCookie, doneChan, vehicleConn, t)
	connectToVehicle(sessionCookie.Value, "bob", newUserCookie, doneChan, vehicleConn, t)

	// admin should be able to just take control at any time if they're viewing
	giveUpControlURL := fmt.Sprintf("%s%s/vehicle/%s/relinquish-control", server.URL, baseAPIURI, sessionCookie.Value)
	req, err := makeReq(server.URL, giveUpControlURL, "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)

	requestURL := fmt.Sprintf("%s%s/vehicle/%s/request-control", server.URL, baseAPIURI, sessionCookie.Value)
	req, err = makeReq(server.URL, requestURL, "GET", nil, newUserCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)

	giveUpControlURL = fmt.Sprintf("%s%s/vehicle/%s/relinquish-control", server.URL, baseAPIURI, sessionCookie.Value)
	req, err = makeReq(server.URL, giveUpControlURL, "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
}

func createVehicle(url string, adminCookie *http.Cookie, t *testing.T) *teleop.VehicleAuthView {
	client := http.Client{}
	generateURL := fmt.Sprintf("%s/vehicle/generate", baseAPIURI)
	req, err := makeReq(url, generateURL, "POST", nil, adminCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	var tokenAuth *teleop.VehicleAuthView
	err = json.NewDecoder(resp.Body).Decode(&tokenAuth)
	require.NoError(t, err)

	return tokenAuth
}

func connectToVehicle(sessionID, clientID string, cookie *http.Cookie, doneChan chan struct{}, vehicleConn *websocket.Conn, t *testing.T) {

	authHeader := http.Header{}
	authHeader.Add("Cookie", cookie.String())
	subscribeURL := fmt.Sprintf("/api/v1/ws/vehicle/%s/user/%s/subscribe", sessionID, clientID)
	clientConn, err := dialWS(wsServer.URL, subscribeURL, authHeader)
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
}

func loginNewUser(url, userName, pass string, t *testing.T) *http.Cookie {
	client := http.Client{}
	registerURL := fmt.Sprintf("%s/register", baseAPIURI)
	authPayload := &teleop.UserAuth{
		UserName: userName,
		Password: pass,
	}
	req, err := makeReq(url, registerURL, "POST", authPayload, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, resp.StatusCode, http.StatusOK)

	return authenticateUser(url, t, authPayload)
}

func authenticateUser(url string, t *testing.T, auth *teleop.UserAuth) *http.Cookie {
	client := http.Client{}
	loginURL := fmt.Sprintf("%s/login", baseAPIURI)
	req, err := makeReq(url, loginURL, "POST", auth, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusOK, resp.StatusCode)

	require.Len(t, resp.Cookies(), 1)
	return resp.Cookies()[0]
}
