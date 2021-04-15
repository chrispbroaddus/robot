package data

import (
	"errors"
	"fmt"
	"time"

	"github.com/garyburd/redigo/redis"
)

const (
	simSessionKeyFmt = "sim-session/%s"
)

var (
	// ErrNoSimSession is returned if a sim session is not found for an id
	ErrNoSimSession = errors.New("there is no sim session for the provided id")
)

// SimulatorState is used to tell the state of a simulator session
type SimulatorState uint32

const (
	// Starting is used when initialized
	Starting SimulatorState = iota
	// Running the simulator is successfully running
	Running
	// Stopped the simulator is stopped not due to an error state
	Stopped
	// Restarting unexpectedly died and is now spinning back up
	Restarting
	// Failed simulator died due to an error
	Failed
)

// SimulatorSession data structure for a given session and how it is stored in storage
type SimulatorSession struct {
	SimulatorID  string         `json:"id"`
	Name         string         `json:"name"`
	OwnerID      string         `json:"owner,omitempty"`
	State        SimulatorState `json:"state"`
	RecipeBookID string         `json:"recipe_id"`
	WebhookURL   string         `json:"webhook_url"`
}

// SetSimSession is the PersistentStorage implementation for setting a new simsession in redis
func (p *PersistentStorage) SetSimSession(s *SimulatorSession) error {
	defer func(now time.Time) {
		requestTime.With("method", setSimSession, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	key := fmt.Sprintf(simSessionKeyFmt, s.SimulatorID)
	err := p.setRedisJSON("SET", key, "", s)
	if err != nil {
		return err
	}

	return nil
}

// UpdateSimSessionState is the PersistentStorage implementation for updating a simulators session in redis
func (p *PersistentStorage) UpdateSimSessionState(simID string, state SimulatorState) error {
	defer func(now time.Time) {
		requestTime.With("method", updateSimSessionState, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	session, err := p.FindSimSession(simID)
	if err != nil {
		if err == redis.ErrNil {
			return ErrNoSimSession
		}

		return err
	}
	session.State = state

	return p.SetSimSession(session)
}

// FindSimSession is the PersistentStorage implementation for getting a simulator session from redis
func (p *PersistentStorage) FindSimSession(simID string) (*SimulatorSession, error) {
	defer func(now time.Time) {
		requestTime.With("method", findSimSession, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	key := fmt.Sprintf(simSessionKeyFmt, simID)
	simSession := new(SimulatorSession)
	err := p.getRedisJSON("GET", key, "", simSession)
	if err != nil {
		if err == redis.ErrNil {
			return nil, ErrNoSimSession
		}

		return nil, err
	}

	return simSession, nil
}

// RemoveSimSession is the PersistentStorage implementation for clearing a simulator session from redis
func (p *PersistentStorage) RemoveSimSession(simID string) error {
	defer func(now time.Time) {
		requestTime.With("method", removeSimSession, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	conn := p.pool.Get()
	defer conn.Close()

	key := fmt.Sprintf(simSessionKeyFmt, simID)
	_, err := conn.Do("DEL", key)
	if err != nil {
		return err
	}

	return nil
}

// SetSimSession is the localSimHostStorage implementation for setting a new simsession in redis
func (l *localSimHostStorage) SetSimSession(s *SimulatorSession) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	l.simSessions[s.SimulatorID] = s

	return nil
}

// UpdateSimSessionState is the localSimHostStorage PersistentStorage implementation for updating a simulators session in redis
func (l *localSimHostStorage) UpdateSimSessionState(simID string, state SimulatorState) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	simSession, ok := l.simSessions[simID]
	if !ok {
		return ErrNoSimSession
	}

	simSession.State = state

	l.simSessions[simID] = simSession

	return nil
}

// FindSimSession is the localSimHostStorage implementation for getting a simulator session from redis
func (l *localSimHostStorage) FindSimSession(simID string) (*SimulatorSession, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	simSession, ok := l.simSessions[simID]
	if !ok {
		return nil, ErrNoSimSession
	}
	return simSession, nil
}

// RemoveSimSession is the localSimHostStorage implementation for clearing a simulator session from redis
func (l *localSimHostStorage) RemoveSimSession(simID string) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	delete(l.simSessions, simID)

	return nil
}

// MarshalText is a json Marshaler implementation for SimulatorState
func (s SimulatorState) MarshalText() ([]byte, error) {
	switch s {
	case Starting:
		return []byte("starting"), nil
	case Running:
		return []byte("running"), nil
	case Stopped:
		return []byte("stopped"), nil
	case Restarting:
		return []byte("restarting"), nil
	case Failed:
		return []byte("Failed"), nil
	default:
		return nil, fmt.Errorf("invalid SimulatorStatus: %d", s)
	}
}

// UnmarshalText is a json Unmarshaler implementation for SimulatorState
func (s *SimulatorState) UnmarshalText(b []byte) error {
	switch string(b) {
	case "starting":
		*s = Starting
		return nil
	case "running":
		*s = Running
		return nil
	case "stopped":
		*s = Stopped
		return nil
	case "restarting":
		*s = Restarting
		return nil
	case "failed":
		*s = Failed
		return nil
	default:
		return fmt.Errorf("cannot parse %s as vehicle status", string(b))
	}
}
