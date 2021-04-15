package services

import (
	"fmt"

	uuid "github.com/satori/go.uuid"
	"github.com/zippyai/zippy/backend/data"
)

// CommunicationType is an enum for the types of ways a user can interact with the sim
type CommunicationType uint32

const (
	// ConnectOut is used if you want some side container to connect to a remote api to use the sim
	ConnectOut CommunicationType = iota
	// ReadFrom is used when you want to just read directly from the frame publisher of the sim
	ReadFrom
	// WebRTC is selected if you want to connect to the sim via webrtc directly
	WebRTC
)

// SimulatorSession is the main actionable piece of a running simulator
type SimulatorSession struct {
	Name              string              `json:"name"`
	ID                string              `json:"-"`
	OwnerID           string              `json:"owner,omitempty"`
	State             data.SimulatorState `json:"state,omitempty"`
	RecipeBookID      string              `json:"recipe_id"`
	StatusWebhook     string              `json:"status_webhook"`
	CommunicationType CommunicationType   `json:"communication_type"`
	ConnectOut        *ExternalConnection `json:"external_connection,omitempty"`
	ReadAddress       *ReadAddress        `json:"read_address,omitempty"`
}

// ExternalConnection is used when you want a program running with the sim to connect to a resource to use the sim
type ExternalConnection struct {
	Domain string `json:"domain"`
	URI    string `json:"uri,omitempty"`
}

// ReadAddress is used when you want to read the sim data from a specific port, the ip will come when a running status is sent on webhook
type ReadAddress struct {
	Port int `json:"port"`
}

// SimulatorStarts is a view struct that is returned when you request a new simulator to start
type SimulatorStarts struct {
	SimulatorID string              `json:"id"`
	State       data.SimulatorState `json:"state"`
}

// MarshalText is a json marshal implementation for CommunicationType
func (c CommunicationType) MarshalText() ([]byte, error) {
	switch c {
	case ConnectOut:
		return []byte("connect_out"), nil
	case ReadFrom:
		return []byte("read_from"), nil
	case WebRTC:
		return []byte("webrtc"), nil
	default:
		return nil, fmt.Errorf("inavlid CommunicationType: %d", c)
	}
}

// UnmarshalText is a json Unmarshaler implementation for CommunicationType
func (c *CommunicationType) UnmarshalText(b []byte) error {
	switch string(b) {
	case "connect_out":
		*c = ConnectOut
		return nil
	case "read_from":
		*c = ReadFrom
		return nil
	case "webrtc":
		*c = WebRTC
		return nil
	default:
		return fmt.Errorf("cannot parse %s as communication type", string(b))
	}
}

// StatusString returns the marshaled string of a session status
func (s *SimulatorSession) StatusString() (string, error) {
	blob, err := s.State.MarshalText()
	if err != nil {
		return "", err
	}

	return string(blob), nil
}

// CreateResourceAlloc returns an unvalidated resource alloc requirements for what is need to run
func (s *SimulatorSession) CreateResourceAlloc(store data.SimulatorHostStorage) (*data.ResourceAlloc, error) {
	recipeBook, err := store.FindRecipeBook(s.RecipeBookID)
	if err != nil {
		return nil, err
	}

	return recipeBook.BuildResourceReq(s.ID), nil
}

// RegisterSession stores a new session in cache for later retreval or updating
func RegisterSession(store data.SimulatorHostStorage, s *SimulatorSession) error {
	s.ID = uuid.NewV4().String()

	storeSession := &data.SimulatorSession{
		SimulatorID:  s.ID,
		Name:         s.Name,
		OwnerID:      s.OwnerID,
		RecipeBookID: s.RecipeBookID,
		State:        data.Starting,
		WebhookURL:   s.StatusWebhook,
	}

	return store.SetSimSession(storeSession)
}

// RunSimulator finds the hosts to place the simulators resources on and places them
func RunSimulator(store data.SimulatorHostStorage, alloc ResourceAllocator, simInfo *SimulatorSession) error {
	resourceAllocation, err := simInfo.CreateResourceAlloc(store)
	if err != nil {
		simInfo.State = data.Failed
		return err
	}

	// find the right hosts and connections needed to spin up the full session
	placementPlan := alloc.GetPlacements(resourceAllocation)

	// execute on the placement plan to start up all of the containers
	err = alloc.PlaceContainers(store, placementPlan)
	if err != nil {
		simInfo.State = data.Failed
		return err
	}
	simInfo.State = data.Starting

	return nil
}

// ClearSimSession gets the latest version of the sim session and removes it from cache
func ClearSimSession(store data.SimulatorHostStorage, sessionID string) (*data.SimulatorSession, error) {
	simInfo, err := store.FindSimSession(sessionID)
	if err != nil {
		return nil, err
	}
	simInfo.State = data.Stopped

	err = store.RemoveSimSession(sessionID)
	if err != nil {
		return nil, err
	}

	return simInfo, nil
}
