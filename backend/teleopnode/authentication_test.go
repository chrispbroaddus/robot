package teleopnode

import (
	"bytes"
	"fmt"
	"testing"
	"time"

	"net/http"
	"net/url"

	"encoding/json"

	uuid "github.com/satori/go.uuid"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"github.com/zippyai/zippy/backend/services"
	"github.com/zippyai/zippy/backend/teleop"
)

func TestAuthentication_RegisterFailure(t *testing.T) {
	registerURL, err := router.Get("RegisterUser").URL()
	require.NoError(t, err)

	// request fails because the json body is malformed
	req, err := makeReq(server.URL, registerURL.String(), "POST", "{:}", nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusBadRequest, resp.StatusCode)

	// request fails because there is no information in the request
	authPayload := &teleop.UserAuth{}
	req, err = makeReq(server.URL, registerURL.String(), "POST", authPayload, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusBadRequest, resp.StatusCode)
}

func TestAuthentication_LoginFailure(t *testing.T) {
	loginURL := fmt.Sprintf("%s/login", baseAPIURI)
	// request fails because the json body is malformed
	req, err := makeReq(server.URL, loginURL, "POST", "{:}", nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusBadRequest, resp.StatusCode)

	// request fails because there is no information in the request
	authPayload := &teleop.UserAuth{}
	req, err = makeReq(server.URL, loginURL, "POST", authPayload, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusNotFound, resp.StatusCode)
}

func TestAuthentication_PromoteUserFailure(t *testing.T) {
	// request will fail because a user doesn't exist for that id
	promoteURL := fmt.Sprintf("%s/user/%s/promote", baseAPIURI, "bad_id")
	req, err := makeReq(server.URL, promoteURL, "POST", nil, adminCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusBadRequest, resp.StatusCode)
}

func TestAuthentication_AuthLifeCycle(t *testing.T) {
	registerURL, err := router.Get("RegisterUser").URL()
	require.NoError(t, err)
	authPayload := &teleop.UserAuth{
		UserName: "bob",
		Password: "builder",
	}
	req, err := makeReq(server.URL, registerURL.String(), "POST", authPayload, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, resp.StatusCode, http.StatusOK)

	loginURL := fmt.Sprintf("%s/login", baseAPIURI)
	req, err = makeReq(server.URL, loginURL, "POST", authPayload, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusOK, resp.StatusCode)
	assert.Len(t, resp.Cookies(), 1)
	authCookie := resp.Cookies()[0]
	assert.Equal(t, services.AuthKey, authCookie.Name)
	// make sure that the expiration on the cookie is set to 2 weeks from now with in the margin of error of the time out time
	assert.WithinDuration(t, time.Now().Add(time.Hour*14*24), authCookie.Expires, time.Duration(time.Second*10))

	logoutURL := fmt.Sprintf("%s/logout", baseAPIURI)
	req, err = makeReq(server.URL, logoutURL, "GET", nil, authCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, resp.StatusCode, http.StatusOK)
	assert.Len(t, resp.Cookies(), 1)
	deleteCookie := resp.Cookies()[0]
	assert.Equal(t, services.AuthKey, deleteCookie.Name)
	assert.Equal(t, -1, deleteCookie.MaxAge)
}

func TestAuthentication_PromoteUser(t *testing.T) {
	// create a new user to promote
	registerURL := fmt.Sprintf("%s/register", baseAPIURI)
	authPayload := &teleop.UserAuth{
		UserName: "bob",
		Password: "builder",
	}
	req, err := makeReq(server.URL, registerURL, "POST", authPayload, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, resp.StatusCode, http.StatusOK)

	var userView *teleop.UserView
	err = json.NewDecoder(resp.Body).Decode(&userView)
	require.NoError(t, err)
	assert.Equal(t, authPayload.UserName, userView.UserName)
	assert.NotZero(t, userView.UserID)

	promoteURL := fmt.Sprintf("%s/user/%s/promote", baseAPIURI, userView.UserID)
	req, err = makeReq(server.URL, promoteURL, "POST", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)

	err = services.IsAdmin(app.storage, userView.UserID)
	require.NoError(t, err)

	// should not be able to promote user when there is no auth
	req, err = makeReq(server.URL, promoteURL, "POST", nil, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusUnauthorized, resp.StatusCode)
}

func TestAuthentication_GenerateVehicleToken(t *testing.T) {
	adminAuth := &teleop.UserAuth{
		UserName: "admin",
		Password: "admin",
	}
	loginURL := fmt.Sprintf("%s/login", baseAPIURI)
	req, err := makeReq(server.URL, loginURL, "POST", adminAuth, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	assert.Len(t, resp.Cookies(), 1)
	authCookie := resp.Cookies()[0]

	generateURL := fmt.Sprintf("%s/vehicle/generate", baseAPIURI)
	req, err = makeReq(server.URL, generateURL, "POST", nil, authCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	var tokenAuth *teleop.VehicleAuthView
	err = json.NewDecoder(resp.Body).Decode(&tokenAuth)
	require.NoError(t, err)

	// try regenerating auth token for a vehicle that already exists
	generateURL = fmt.Sprintf("%s?vehicle_id=%s", generateURL, tokenAuth.VehicleID)
	req, err = makeReq(server.URL, generateURL, "POST", nil, authCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	var newTokenAuth *teleop.VehicleAuthView
	err = json.NewDecoder(resp.Body).Decode(&newTokenAuth)
	require.NoError(t, err)
	assert.Equal(t, tokenAuth.VehicleID, newTokenAuth.VehicleID)
	assert.Equal(t, tokenAuth.Token, newTokenAuth.Token)
}

func TestAuthentication_SetTokenValidityFailure(t *testing.T) {
	vehicleID := uuid.NewV4().String()
	validateURL := fmt.Sprintf("%s/vehicle/token/%s/validate", baseAPIURI, vehicleID)
	req, err := makeReq(server.URL, validateURL, "PUT", nil, adminCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusBadRequest, resp.StatusCode)
}

func TestAuthentication_FailedLogin(t *testing.T) {
	adminAuth := &teleop.UserAuth{
		UserName: "test",
		Password: "invalid",
	}
	loginURL := fmt.Sprintf("%s/login", baseAPIURI)
	req, err := makeReq(server.URL, loginURL, "POST", adminAuth, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusNotFound, resp.StatusCode)
}

func makeReq(baseURL, path, method string, payload interface{}, cookie *http.Cookie) (*http.Request, error) {
	var err error
	var body []byte
	if payload != nil {
		body, err = json.Marshal(payload)
		if err != nil {
			return nil, err
		}
	}

	reqURL, err := url.Parse(baseURL)
	if err != nil {
		return nil, err
	}

	reqURL, err = reqURL.Parse(path)
	if err != nil {
		return nil, err
	}

	req, err := http.NewRequest(method, reqURL.String(), bytes.NewReader(body))
	if err != nil {
		return nil, err
	}

	if cookie != nil {
		req.AddCookie(cookie)
	}

	return req, nil
}

func loginUser(url string, auth *teleop.UserAuth) (*http.Cookie, error) {
	loginURL := fmt.Sprintf("%s/login", baseAPIURI)
	req, err := makeReq(url, loginURL, "POST", auth, nil)
	if err != nil {
		return nil, err
	}

	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}

	for _, cookie := range resp.Cookies() {
		if cookie.Name == services.AuthKey {
			return cookie, nil
		}
	}

	return nil, fmt.Errorf("no auth cookie found")
}
