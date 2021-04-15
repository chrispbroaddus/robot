package teleopnode

import (
	"context"
	"errors"
	"log"
	"sync"
	"time"

	"github.com/gorilla/websocket"
	uuid "github.com/satori/go.uuid"
	"github.com/zippyai/zippy/backend/data"

	teleopproto "github.com/zippyai/zippy/packages/teleop/proto"
)

type listenerType int

// listener types to route vehicle messages to the right queues
const (
	webrtc listenerType = iota
	location
	camera
	control
	view
	drop
)

const (
	// buffer size for channel for sending messages to operator
	operatorQueue = 1
	// time to wait from a network disconnect of the vehicle before telling clients
	listenerTimeout = time.Second * 60
)

var (
	// ErrNoListener is returned if you try and stop listening for an id that doesn't exist
	ErrNoListener = errors.New("there is no listener for the id requested")
)

// relay holds a connection to one vehicle and zero or more operators.
type relay struct {
	mu              sync.Mutex
	id              string
	vehicle         *vehicleConn
	networkNotifier chan bool
	listeners       map[listenerType]map[string]*vehicleListener
}

// vehicleListener are used to be able to relay messages from a vehicle to the client groups that are listening
// the cancel context here is used to break out of the listening client websocket connection when the vehicle goes away
type vehicleListener struct {
	id     string
	cancel context.CancelFunc
	ch     chan *teleopproto.VehicleMessage
}

type vehicleConn struct {
	ws     *websocket.Conn
	ch     chan *teleopproto.BackendMessage
	cancel context.CancelFunc
}

// listen returns a channel that will receive all future messages from the vehicle
func (r *relay) listen(listenerType listenerType, cancel context.CancelFunc) *vehicleListener {
	r.mu.Lock()
	defer r.mu.Unlock()

	// initialize map if it doesn't exist yet
	listeners, ok := r.listeners[listenerType]
	if !ok {
		listeners = make(map[string]*vehicleListener)
	}

	// create new listener and register it
	id := uuid.NewV4().String()
	newListener := make(chan *teleopproto.VehicleMessage, operatorQueue)
	vehicleListener := &vehicleListener{
		id:     id,
		cancel: cancel,
		ch:     newListener,
	}

	listeners[id] = vehicleListener
	r.listeners[listenerType] = listeners

	return vehicleListener
}

// unlisten removes a listener from the relay given an id
func (r *relay) unlisten(id string) error {
	r.mu.Lock()
	defer r.mu.Unlock()

	for _, listeners := range r.listeners {
		listener, ok := listeners[id]
		if ok {
			// close the channel and delete the record form the map
			close(listener.ch)
			delete(listeners, id)
			return nil
		}
	}

	return ErrNoListener
}

func (r *relay) notifyVehicleStateChange(state data.VehicleStatus) {
	if state == data.StatusOffline {
		r.killListeners()
		return
	}

	go func() {
		timeout := time.NewTimer(listenerTimeout)
		defer timeout.Stop()
		for {
			select {
			case <-timeout.C:
				r.killListeners()
				return
			case <-r.networkNotifier: // new vehicle session was made and websocket is back
				return
			}
		}
	}()
}

func (r *relay) killListeners() {
	r.mu.Lock()
	defer r.mu.Unlock()

	for _, listeners := range r.listeners {
		for _, listener := range listeners {
			listener.cancel()
		}
	}
}

func (r *relay) notifyType(listenerType listenerType, msg *teleopproto.VehicleMessage) {
	r.mu.Lock()
	defer r.mu.Unlock()

	// we do not want to send these messages to anyone
	if listenerType == drop {
		return
	}

	listeners, ok := r.listeners[listenerType]
	if !ok {
		// we can't send to anyone if there's no one registered yet
		return
	}

	log.Printf("sending message to %d listeners", len(listeners))
	for id, listener := range listeners {
		select {
		case listener.ch <- msg:
		default:
			log.Printf("unable to notify listener with id: %s since the channel is busy\n", id)
		}
	}
}

func (r *relay) setVehicle(
	conn *websocket.Conn,
	ch chan *teleopproto.BackendMessage,
	cancel context.CancelFunc) {

	if r.vehicle != nil {
		// notify any waiting reconnects that you have reconnected
		r.networkNotifier <- true
		// clear the previous vehicle if it hasn't been cleared yet
		r.clearVehicle()
	}

	r.mu.Lock()
	defer r.mu.Unlock()
	// recreate the notifier so any dead messages don't block
	if r.networkNotifier != nil {
		close(r.networkNotifier)
	}
	r.networkNotifier = make(chan bool)

	r.vehicle = &vehicleConn{
		ws:     conn,
		ch:     ch,
		cancel: cancel,
	}
}

func (r *relay) clearVehicle() {
	r.mu.Lock()
	defer r.mu.Unlock()

	if r.vehicle != nil {
		r.vehicle.ws.Close()
		close(r.vehicle.ch)
		r.vehicle.cancel()
	}

	r.vehicle = nil
}

func (r *relay) processCommand(msg *teleopproto.BackendMessage) {
	r.mu.Lock()
	defer r.mu.Unlock()

	if r.vehicle == nil {
		return
	}

	log.Println("sending message to vehicle")
	select {
	case r.vehicle.ch <- msg:
	default:
		log.Println("vehicle queue full, dropping message")
	}
}
