package services

import (
	"errors"
	"fmt"
	"sync"

	"github.com/zippyai/zippy/backend/data"
	"golang.org/x/crypto/ssh"
)

var (
	// ErrRecipeNotInBook recipe not found in book
	ErrRecipeNotInBook = errors.New("the recipe step you want to deploy is not in the recipeBook")
	// ErrContainerCouldNotBePlaced could not place container on any available hosts
	ErrContainerCouldNotBePlaced = errors.New("a container from the deploy plan was unable to be placed on any host")
	// ErrSessionHasNoContainers there are no containers left to stop for a session
	ErrSessionHasNoContainers = errors.New("the session you're trying to clean up has no containers")
)

const (
	killCmdBase = "sudo docker kill "
)

// ResourceAllocator interface for spinning up simulators on any given host configuration
type ResourceAllocator interface {
	// GetPlacements is used to take a given ReciepBooks RecourceAlloc and come up with a placement plan to run each container on a given host that has enough resources to run it
	GetPlacements(resourceAlloc *data.ResourceAlloc) *PlacementPlan
	// PlaceContainers takes a Placement plan that has been approved and starts all of the containers on the given hosts that they are alloted to
	PlaceContainers(store data.SimulatorHostStorage, plan *PlacementPlan) error
	// RemoveSession kill all of the containers that are used for this simulator run
	RemoveSession(sessionID string) error
}

// PlacementPlan is a generic plan of an entire simulator session that is in the stages of deployment
type PlacementPlan struct {
	sessionID    string
	recipeBookID string
	containers   []*ContainerPlacement
}

// ContainerPlacement is a generic struct to contain the information deployed for a given recipe step
type ContainerPlacement struct {
	recipeID        string
	availableHosts  []string
	hostsContainers map[string][]string
}

type sshAgents struct {
	connections map[string]*ssh.Client
}

// SSHResourceAllocator ResourceAllocator that uses an ssh client to the hosts in order to start sims
type SSHResourceAllocator struct {
	mu              sync.Mutex
	username        string
	simServers      []string
	keyFile         string
	runningSessions map[string][]*ContainerPlacement
}

// NewSSHResourceAllocator creates a new SSHResourceAllocator that implemnents the ResourceAllocator interface
func NewSSHResourceAllocator(username, keyFile string, simServers ...string) ResourceAllocator {
	return &SSHResourceAllocator{
		username:        username,
		simServers:      simServers,
		keyFile:         keyFile,
		runningSessions: make(map[string][]*ContainerPlacement),
	}
}

// GetPlacements creates a PlacementPlan to run the containers needed and on which hosts
// right now this just uses on host but should eventually check hosts to see which it can place onto for which containers
func (s *SSHResourceAllocator) GetPlacements(resourceAlloc *data.ResourceAlloc) *PlacementPlan {
	plan := &PlacementPlan{
		recipeBookID: resourceAlloc.RecipeBookID,
		sessionID:    resourceAlloc.SessionID,
	}

	for recipeID := range resourceAlloc.DeploymentReqs {
		plan.containers = append(plan.containers, &ContainerPlacement{
			recipeID: recipeID,
			// when we add more hosts this will need to be dynamic based on load and service discovery
			hostsContainers: make(map[string][]string),
			availableHosts:  s.simServers,
		})
	}

	return plan
}

// PlaceContainers takes a given ResourceAlloc and places all of the recipes on the selected hosts
func (s *SSHResourceAllocator) PlaceContainers(store data.SimulatorHostStorage, plan *PlacementPlan) error {
	recipeBook, err := store.FindRecipeBook(plan.recipeBookID)
	if err != nil {
		return err
	}

	bookCommands := recipeBook.GatherPlacementCommands()

	for _, container := range plan.containers {
		cmds, ok := bookCommands[container.recipeID]
		if !ok {
			return ErrRecipeNotInBook
		}

		placementAgent, err := s.createSSHAgents(container.availableHosts...)
		if err != nil {
			return err
		}

		for _, cmd := range cmds {
			// try every available ssh agent host before giving up and throwing an error
			deployment, err := placementAgent.deployContainer(cmd)
			if err != nil {
				return err
			}

			// keep track of the containerID that was created
			container.hostsContainers[deployment.host] = append(container.hostsContainers[deployment.host], deployment.containerID)
		}

		placementAgent.close()
	}

	s.mu.Lock()
	defer s.mu.Unlock()
	s.runningSessions[plan.sessionID] = plan.containers

	return nil
}

// RemoveSession kills all of the containers from a given session
func (s *SSHResourceAllocator) RemoveSession(sessionID string) error {
	runningContainers, err := s.getContainers(sessionID)
	if err != nil {
		return err
	}

	for _, container := range runningContainers {
		err := s.killAll(container)
		if err != nil {
			return err
		}
	}

	s.mu.Lock()
	defer s.mu.Unlock()
	delete(s.runningSessions, sessionID)

	return nil
}

func (s *SSHResourceAllocator) getContainers(sessionID string) ([]*ContainerPlacement, error) {
	s.mu.Lock()
	defer s.mu.Unlock()

	runningContainers, ok := s.runningSessions[sessionID]
	if !ok {
		return nil, ErrSessionHasNoContainers
	}

	return runningContainers, nil
}

func (s *SSHResourceAllocator) createSSHAgents(hosts ...string) (*sshAgents, error) {
	clientMap := make(map[string]*ssh.Client)

	for _, host := range hosts {
		client, err := s.newSSHClient(host)
		if err != nil {
			return nil, err
		}

		clientMap[host] = client
	}

	return &sshAgents{
		connections: clientMap,
	}, nil
}

func (s *SSHResourceAllocator) newSSHClient(hostIP string) (*ssh.Client, error) {
	return createSSHClient(s.username, s.keyFile, hostIP)
}

type deployment struct {
	containerID string
	host        string
}

func (s *sshAgents) deployContainer(cmd string) (*deployment, error) {
	for host, client := range s.connections {
		session, err := client.NewSession()
		if err != nil {
			// if we're  unable to place container on this host try the next
			continue
		}
		defer session.Close()

		respBlob, err := session.Output(cmd)
		if err != nil {
			// if we're  unable to place container on this host try the next
			continue
		}

		return &deployment{
			containerID: string(respBlob),
			host:        host,
		}, nil
	}

	// if we were unable to place a container on any hosts than the deploy failed
	return nil, ErrContainerCouldNotBePlaced
}

func (s *sshAgents) close() {
	for _, client := range s.connections {
		client.Close()
	}
}

func (s *SSHResourceAllocator) killAll(c *ContainerPlacement) error {
	for host, containerIDs := range c.hostsContainers {
		sshAgents, err := s.createSSHAgents(host)
		if err != nil {
			return err
		}

		agent := sshAgents.connections[host]
		session, err := agent.NewSession()
		if err != nil {
			return err
		}

		cmd := createKillCmd(containerIDs...)

		if err := session.Run(cmd); err != nil {
			return err
		}

		agent.Close()
	}

	return nil
}

func createKillCmd(containerIDs ...string) string {
	resultCmd := killCmdBase
	for _, containerID := range containerIDs {
		resultCmd = fmt.Sprintf("%s %s", resultCmd, containerID)
	}

	return resultCmd
}

// TestResourceAllocator is used to dummy the operations of simulator runs
type TestResourceAllocator struct {
	mu              sync.Mutex
	runningSessions map[string][]*ContainerPlacement
}

// NewTestResourceAllocator creates a resource allocator used to dummy the interface
func NewTestResourceAllocator() *TestResourceAllocator {
	return &TestResourceAllocator{
		runningSessions: make(map[string][]*ContainerPlacement),
	}
}

// GetPlacements creates a PlacementPlan to run the containers needed and on which hosts
func (t *TestResourceAllocator) GetPlacements(resourceAlloc *data.ResourceAlloc) *PlacementPlan {
	plan := &PlacementPlan{
		recipeBookID: resourceAlloc.RecipeBookID,
		sessionID:    resourceAlloc.SessionID,
	}

	for recipeID := range resourceAlloc.DeploymentReqs {
		plan.containers = append(plan.containers, &ContainerPlacement{
			recipeID:        recipeID,
			hostsContainers: make(map[string][]string),
			availableHosts:  []string{"test_host"},
		})
	}

	return plan
}

// PlaceContainers takes a given ResourceAlloc and places all of the recipes on the selected hosts
func (t *TestResourceAllocator) PlaceContainers(store data.SimulatorHostStorage, plan *PlacementPlan) error {
	t.mu.Lock()
	defer t.mu.Unlock()
	t.runningSessions[plan.sessionID] = plan.containers

	return nil
}

// RemoveSession kills all of the containers from a given session
func (t *TestResourceAllocator) RemoveSession(sessionID string) error {
	t.mu.Lock()
	defer t.mu.Unlock()
	delete(t.runningSessions, sessionID)

	return nil
}
