package teleopnode

import (
	"context"
	"encoding/json"
	"fmt"
	"net/http"
	"sync"
	"testing"

	"github.com/zippyai/zippy/backend/data"
	"github.com/zippyai/zippy/backend/teleop"

	"github.com/stretchr/testify/require"
)

type mockSimController struct {
	mu          sync.Mutex
	runningSims map[string]*testMessanger
}

// testMessager act as a simulatorReporter for communicating between concurrent sim operations
type testMessanger struct {
	ctx     context.Context
	cancel  context.CancelFunc
	msgChan chan interface{}
}

// test implementation does not need to ssh and actually run sims just wait for a kill command
func (t *mockSimController) Run(ctx context.Context, name string, simulator *data.Simulator) error {
	t.mu.Lock()

	messenger, ok := t.runningSims[name]
	if !ok {
		childCtx, cancelFunc := context.WithCancel(ctx)
		messenger = &testMessanger{
			ctx:    childCtx,
			cancel: cancelFunc,
		}
	}

	t.runningSims[name] = messenger
	t.mu.Unlock()

	<-messenger.ctx.Done()

	return nil
}

// test implementation does not need to do anything but kill the run go routine
func (t *mockSimController) Close(simulatorName string) error {
	t.mu.Lock()
	defer t.mu.Unlock()

	messenger, ok := t.runningSims[simulatorName]
	if !ok {
		return fmt.Errorf("simulator is already closed")
	}

	messenger.cancel()
	delete(t.runningSims, simulatorName)
	return nil
}

func (t *mockSimController) RegisterListener(id string) chan interface{} {
	t.mu.Lock()
	defer t.mu.Unlock()

	messenger, ok := t.runningSims[id]
	if !ok {
		messenger = &testMessanger{}
	}

	if messenger.msgChan == nil {
		messenger.msgChan = make(chan interface{})
	}
	t.runningSims[id] = messenger

	return messenger.msgChan
}

func (t *mockSimController) Notify(id string, msg interface{}) error {
	t.mu.Lock()
	defer t.mu.Unlock()

	messenger, ok := t.runningSims[id]
	if !ok {
		return ErrNoListener
	}

	if messenger.msgChan == nil {
		return fmt.Errorf("no message channel has been created yet")
	}

	select {
	case messenger.msgChan <- msg:
	default:
		return fmt.Errorf("unable to send message")
	}

	return nil
}

func TestSimulator_SimTypesFailure(t *testing.T) {
	// request failed because of bad auth
	getSimsURL := "/api/v1/simulators"
	req, err := makeReq(server.URL, getSimsURL, "GET", nil, badCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusUnauthorized, resp.StatusCode)
}

func TestSimulator_StartSimulatorFailure(t *testing.T) {
	// request failed because of bad auth
	startSimsURL := "/api/v1/simulator/start"
	req, err := makeReq(server.URL, startSimsURL, "POST", nil, badCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusUnauthorized, resp.StatusCode)

	// request failed due to malformed json body
	req, err = makeReq(server.URL, startSimsURL, "POST", "{:}", adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusBadRequest, resp.StatusCode)

	payload := &teleop.StartSimulator{
		SimulatorType: "bad_type",
		VehicleName:   "name",
	}

	// request failed due to no simulator of that type
	req, err = makeReq(server.URL, startSimsURL, "POST", payload, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusBadRequest, resp.StatusCode)
}

func TestSimulator_KillSimulatorFailure(t *testing.T) {
	vehicleName := "sim_name"
	killSimURL := fmt.Sprintf("/api/v1/simulator/%s/kill", vehicleName)
	// request failed due to no sim running with that name
	req, err := makeReq(server.URL, killSimURL, "PUT", nil, adminCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusBadRequest, resp.StatusCode)

	// request failed because of bad auth
	req, err = makeReq(server.URL, killSimURL, "PUT", nil, badCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusUnauthorized, resp.StatusCode)
}

func TestSimulator_LifeCycle(t *testing.T) {
	doneChan := setDoneListener()
	defer func() {
		done = func() {}
	}()

	getSimsURL := "/api/v1/simulators"
	req, err := makeReq(server.URL, getSimsURL, "GET", nil, adminCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	defer resp.Body.Close()

	var simTypes []string
	err = json.NewDecoder(resp.Body).Decode(&simTypes)
	require.NoError(t, err)
	require.Len(t, simTypes, 1)

	vehicleName := "test_vehicle"
	newSim := &teleop.StartSimulator{
		SimulatorType: simTypes[0],
		VehicleName:   vehicleName,
	}
	startSimURL := "/api/v1/simulator/start"
	req, err = makeReq(server.URL, startSimURL, "POST", newSim, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusOK, resp.StatusCode)

	notifierConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/simulator/%s/notifier", vehicleName), authHeader)
	require.NoError(t, err)
	defer notifierConn.Close()
	<-doneChan

	simStarting := new(teleop.SimulatorEvent)
	err = RecvJSON(notifierConn, simStarting)
	require.NoError(t, err)
	require.Equal(t, vehicleName, simStarting.ID)
	require.Equal(t, teleop.Start, simStarting.Status)

	vehicleConn, err := dialWS(wsServer.URL, fmt.Sprintf("/api/v1/ws/vehicle/%s/register", vehicleName), nil)
	require.NoError(t, err)
	defer vehicleConn.Close()
	<-doneChan

	simStarted := new(teleop.SimulatorEvent)
	err = RecvJSON(notifierConn, simStarted)
	require.NoError(t, err)
	require.Equal(t, vehicleName, simStarted.ID)
	require.Equal(t, teleop.Ready, simStarted.Status)
	<-doneChan

	killSimURL := fmt.Sprintf("/api/v1/simulator/%s/kill", vehicleName)
	req, err = makeReq(server.URL, killSimURL, "PUT", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusOK, resp.StatusCode)
}

func TestCreateDeleteSimulatorType(t *testing.T) {
	testSim := &data.Simulator{
		Name:                "test_sim",
		SimulatorConfigFile: "/test/config/simulator_config.json",
	}

	createSimURL := "/api/v1/simulator/create"
	req, err := makeReq(server.URL, createSimURL, "POST", testSim, adminCookie)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusOK, resp.StatusCode)

	deleteSimURL := fmt.Sprintf("/api/v1/simulator/%s/delete", testSim.Name)
	req, err = makeReq(server.URL, deleteSimURL, "DELETE", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.Equal(t, http.StatusOK, resp.StatusCode)
}
func TestSimulatorFailCases(t *testing.T) {
	vehicleName := "test_vehicle"
	newSim := &teleop.StartSimulator{
		SimulatorType: "not_a_real_sim",
		VehicleName:   vehicleName,
	}
	startSimURL := "/api/v1/simulator/start"
	// no auth
	req, err := makeReq(server.URL, startSimURL, "POST", newSim, nil)
	require.NoError(t, err)

	resp, err := client.Do(req)
	require.NoError(t, err)
	require.NotEqual(t, http.StatusOK, resp.StatusCode)

	// invalid input
	req, err = makeReq(server.URL, startSimURL, "POST", []byte("bad json"), nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.NotEqual(t, http.StatusOK, resp.StatusCode)

	// simulator does not exist
	req, err = makeReq(server.URL, startSimURL, "POST", newSim, nil)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.NotEqual(t, http.StatusOK, resp.StatusCode)

	killSimURL := fmt.Sprintf("/api/v1/simulator/%s/kill", "bad_vehicle_name")
	// kill sim that is not running
	req, err = makeReq(server.URL, killSimURL, "PUT", nil, adminCookie)
	require.NoError(t, err)

	resp, err = client.Do(req)
	require.NoError(t, err)
	require.NotEqual(t, http.StatusOK, resp.StatusCode)
}
