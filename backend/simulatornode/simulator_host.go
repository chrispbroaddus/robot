package simulatornode

import (
	"encoding/json"
	"log"
	"net/http"

	"github.com/gorilla/mux"
	"github.com/zippyai/zippy/backend/configs"
	"github.com/zippyai/zippy/backend/data"
	"github.com/zippyai/zippy/backend/services"
	"github.com/zippyai/zippy/backend/simulator"
)

// SessionManager is used to maintain all of the sim hosts core pointers
type SessionManager struct {
	webhookProc webhookSender
	storage     data.SimulatorHostStorage
	config      *configs.Config
	alloc       services.ResourceAllocator
	env         configs.Environment
}

// NewSessionManager creates the main struct for the sim host from an options struct
func NewSessionManager(o *SimHostOptions) *SessionManager {
	webhookProc := newNetworkSender()

	// start the running loop to process web hook messages
	go webhookProc.start()

	return &SessionManager{
		webhookProc: webhookProc,
		storage:     o.storage,
		config:      o.configs,
		alloc:       services.NewSSHResourceAllocator(o.configs.SSHConfig.Username, o.configs.SSHConfig.KeyFile, o.configs.SSHConfig.SimServerAddr),
		env:         o.env,
	}
}

func (s *SessionManager) startSimulator(w http.ResponseWriter, r *http.Request) {
	pathTmpl, err := mux.CurrentRoute(r).GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	simInfo := new(services.SimulatorSession)
	err = json.NewDecoder(r.Body).Decode(simInfo)
	if err != nil {
		log.Println("unable to unmarshal simulator start json input: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	if err := services.RegisterSession(s.storage, simInfo); err != nil {
		log.Println("unable to register new simulator session: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusInternalServerError)
		return
	}

	var status string
	err = services.RunSimulator(s.storage, s.alloc, simInfo)
	status, err = simInfo.StatusString()
	if err != nil {
		log.Println("unable to start running the simulator simulator: ", err)
		s.webhookProc.send(simInfo.StatusWebhook, simulator.NewStatusMsg(simInfo.ID, status, err.Error()))
		respondError(w, r, err, "", pathTmpl, http.StatusInternalServerError)
		return
	}

	s.webhookProc.send(simInfo.StatusWebhook, simulator.NewStatusMsg(simInfo.ID, status, ""))

	respondJSON(w, r, "", &services.SimulatorStarts{
		SimulatorID: simInfo.ID,
		State:       simInfo.State,
	})
}

func (s *SessionManager) stopSimulator(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	simulatorID := vars["sim_id"]
	pathTmpl, err := mux.CurrentRoute(r).GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	log.Println("killing simulator with id: ", simulatorID)
	defer log.Println("killed simulator with id: ", simulatorID)

	err = s.alloc.RemoveSession(simulatorID)
	if err != nil {
		log.Printf("unable to kill running simulator from the host with id: %s given error: %s", simulatorID, err)
		respondError(w, r, err, "", pathTmpl, http.StatusInternalServerError)
		return
	}

	simInfo, err := services.ClearSimSession(s.storage, simulatorID)
	if err != nil {
		log.Printf("unable to kill running simulator from the host with id: %s given error: %s", simulatorID, err)
		respondError(w, r, err, "", pathTmpl, http.StatusInternalServerError)
		return
	}

	var errMsg string
	statusBlob, err := simInfo.State.MarshalText()
	if err != nil {
		log.Printf("unable to marshal the status change enum: %s error: %s", simInfo.WebhookURL, err)
		errMsg = err.Error()
	}
	s.webhookProc.send(simInfo.WebhookURL, simulator.NewStatusMsg(simulatorID, string(statusBlob), errMsg))

	//respondJSON(w, r, "", nil)
}

func respondJSON(w http.ResponseWriter, r *http.Request, requesterID string, payload interface{}) {
	jBlob, err := json.Marshal(payload)
	if err != nil {
		log.Println("unable to write response: ", err)
		respondError(w, r, err, requesterID, "", http.StatusInternalServerError)
		return
	}

	httpRequestsTotal.With("endpoint", "", "method", r.Method, "requester", requesterID, "code", "200").Add(1)
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	if len(jBlob) > 0 {
		w.Write(jBlob)
	}
}
