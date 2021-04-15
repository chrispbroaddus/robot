package services

import (
	"errors"
	"sync"
)

var (
	// ErrNoListener is returned if there is no listener registered for a given id
	ErrNoListener = errors.New("there is no listener for the id provided")
)

// Notifier is a mockable interface for notifying listeners
type Notifier interface {
	Notify(id string, msg interface{}) error
}

// RegisterNotifier is an interface for registering new notifiers and notifying a given id
type RegisterNotifier interface {
	RegisterListener(id string) chan interface{}
	Notifier
}

// ManagedNotifier is an interface wrapper for Notifier that allows you to add and remove notification listeners
type ManagedNotifier interface {
	DeRegisterListener(id string) error
	RegisterNotifier
}

type internalNotifier struct {
	mu        sync.Mutex
	listeners map[string]chan interface{}
}

// NewInternalManagedNotifier creates a new ManagedNotifier with an internal notifier backing
func NewInternalManagedNotifier() ManagedNotifier {
	return &internalNotifier{
		listeners: make(map[string]chan interface{}),
	}
}

func (i *internalNotifier) Notify(id string, msg interface{}) error {
	i.mu.Lock()
	defer i.mu.Unlock()

	listener, ok := i.listeners[id]
	if !ok {
		return ErrNoListener
	}
	// we need to unlock the mutex before we block on sending the message itself

	go func() {
		listener <- msg
	}()

	return nil
}

func (i *internalNotifier) RegisterListener(id string) chan interface{} {
	i.mu.Lock()
	defer i.mu.Unlock()

	newListener := make(chan interface{})

	i.listeners[id] = newListener

	return newListener
}

func (i *internalNotifier) DeRegisterListener(id string) error {
	i.mu.Lock()
	defer i.mu.Unlock()

	listener, ok := i.listeners[id]
	if !ok {
		return ErrNoListener
	}
	// close the listener channel
	close(listener)
	// remove the record for a channel with the given id
	delete(i.listeners, id)

	return nil
}
