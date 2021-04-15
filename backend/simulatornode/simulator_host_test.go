package simulatornode

import (
	"bytes"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"net/url"
	"os"
	"testing"

	"github.com/zippyai/zippy/backend/data"

	"github.com/adams-sarah/test2doc/test"
	"github.com/gorilla/mux"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	"github.com/zippyai/zippy/backend/services"
	"github.com/zippyai/zippy/backend/simulator"

	"github.com/zippyai/zippy/backend/configs"
)

var (
	webHookChan chan interface{}
	client      http.Client
	server      *test.Server
	manager     *SessionManager
)

func TestMain(m *testing.M) {
	var err error
	manager, err = createTestManager(configs.Local)
	if err != nil {
		panic("unable to create local test app")
	}

	webHookSender := createTestWebhookSender()
	manager.webhookProc = webHookSender
	webHookChan = webHookSender.sendChan
	go webHookSender.start()

	router := manager.NewSimulatorHostRouter()
	fmt.Println(router.Get("register"))
	test.RegisterURLVarExtractor(mux.Vars)
	client = http.Client{}

	server, err = test.NewServer(router)
	if err != nil {
		panic(fmt.Sprintf("error while starting test server: %s", err))
	}

	log.Println("starting tests")
	exitCode := m.Run()

	fmt.Println("server finished and is cleaning up now")
	server.Finish()

	os.Exit(exitCode)
}

var (
	newSim = &services.SimulatorSession{
		Name:              "test_sim",
		StatusWebhook:     "test_webhook",
		CommunicationType: services.WebRTC,
	}
)

func TestSimHost_StartSim(t *testing.T) {
	// create a recipe book first
	req, err := makeReq(server.URL, "/recipe/book/create", "POST", newTestBook, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	respPayload := new(simulator.NewRecipePayload)
	err = json.NewDecoder(resp.Body).Decode(respPayload)
	require.NoError(t, err)
	assert.Equal(t, newTestBook.Name, respPayload.Name)
	assert.Equal(t, newTestBook.Version, respPayload.Version)

	newSim.RecipeBookID = respPayload.ID
	req, err = makeReq(server.URL, "/simulator/start", "POST", newSim, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	simStart := new(services.SimulatorStarts)
	err = json.NewDecoder(resp.Body).Decode(simStart)
	require.NoError(t, err)
	assert.NotEqual(t, "", simStart.SimulatorID)
	assert.Equal(t, data.Starting, simStart.State)
	// read from the webhook processor on status update
	msg := <-webHookChan
	status, ok := msg.(*simulator.StatusView)
	assert.True(t, ok)
	assert.Equal(t, simStart.SimulatorID, status.SessionID)
	blob, err := simStart.State.MarshalText()
	require.NoError(t, err)
	assert.Equal(t, string(blob), status.Status)
	assert.Equal(t, "", status.ErrorMessage)
}

func TestSimHost_RemoveSim(t *testing.T) {
	// create a recipe book first
	req, err := makeReq(server.URL, "/recipe/book/create", "POST", newTestBook, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	respPayload := new(simulator.NewRecipePayload)
	err = json.NewDecoder(resp.Body).Decode(respPayload)
	require.NoError(t, err)
	assert.Equal(t, newTestBook.Name, respPayload.Name)
	assert.Equal(t, newTestBook.Version, respPayload.Version)

	newSim.RecipeBookID = respPayload.ID
	req, err = makeReq(server.URL, "/simulator/start", "POST", newSim, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()

	simStart := new(services.SimulatorStarts)
	err = json.NewDecoder(resp.Body).Decode(simStart)
	require.NoError(t, err)
	assert.NotEqual(t, "", simStart.SimulatorID)
	assert.Equal(t, data.Starting, simStart.State)
	// read from the webhook processor on status update
	msg := <-webHookChan
	status, ok := msg.(*simulator.StatusView)
	assert.True(t, ok)
	assert.Equal(t, simStart.SimulatorID, status.SessionID)
	blob, err := simStart.State.MarshalText()
	require.NoError(t, err)
	assert.Equal(t, string(blob), status.Status)
	assert.Equal(t, "", status.ErrorMessage)

	req, err = makeReq(server.URL, fmt.Sprintf("/simulator/%s/stop", simStart.SimulatorID), "DELETE", nil, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	assert.Equal(t, http.StatusOK, resp.StatusCode)
	defer resp.Body.Close()
	// read the close status update from the webhook
	msg = <-webHookChan
	status, ok = msg.(*simulator.StatusView)
	assert.True(t, ok)
	assert.Equal(t, simStart.SimulatorID, status.SessionID)
	var msgStatus data.SimulatorState
	err = msgStatus.UnmarshalText([]byte(status.Status))
	require.NoError(t, err)
	assert.Equal(t, data.Stopped, msgStatus)
}

type testWebhookSender struct {
	sendChan    chan interface{}
	processChan chan interface{}
}

func (t *testWebhookSender) start() {
	for msg := range t.processChan {
		t.sendChan <- msg
	}
}

func (t *testWebhookSender) send(url string, payload interface{}) {
	t.processChan <- payload
}

func createTestWebhookSender() *testWebhookSender {
	return &testWebhookSender{
		sendChan:    make(chan interface{}),
		processChan: make(chan interface{}),
	}
}
func createTestManager(env configs.Environment) (*SessionManager, error) {
	opts := &SimHostOptions{}
	storage, err := opts.newStorage()
	if err != nil {
		return nil, err
	}

	return &SessionManager{
		config:  &configs.Config{},
		env:     env,
		storage: storage,
		alloc:   services.NewTestResourceAllocator(),
	}, nil
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
