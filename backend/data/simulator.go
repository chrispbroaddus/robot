package data

import (
	"errors"
	"time"

	"github.com/garyburd/redigo/redis"
)

var (
	// ErrNoSimulator is resturned if a simulator is not found for that name
	ErrNoSimulator = errors.New("no simulator was found for that name")
)

const (
	simulatorKey = "simulators"
)

// Simulator contains the information needed to start up the right simulator
type Simulator struct {
	Name                string `json:"name"`
	SimulatorConfigFile string `json:"config_file"`
}

// SimulatorNames is the PersistentStorage implementation to get all of the names of simulators we can start
func (p *PersistentStorage) SimulatorNames() ([]string, error) {
	defer func(now time.Time) {
		requestTime.With("method", simulatorNames, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	conn := p.pool.Get()
	defer conn.Close()

	resultNames, err := redis.Strings(conn.Do("HKEYS", simulatorKey))
	if err != nil {
		if err == redis.ErrNil {
			return nil, ErrNoSimulator
		}

		operationsErrorTotal.With("error", err.Error(), "method", simulatorNames, "data_store", redisStore).Add(1)
		return nil, err
	}

	return resultNames, nil
}

// SimulatorInfo is the PersistentStorage implementation to get the details needed to run a specific server
func (p *PersistentStorage) SimulatorInfo(simName string) (*Simulator, error) {
	defer func(now time.Time) {
		requestTime.With("method", simulatorInfo, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	sim := new(Simulator)
	if err := p.getRedisJSON("HGET", simulatorKey, simName, sim); err != nil {
		if err == redis.ErrNil {
			return nil, ErrNoSimulator
		}

		operationsErrorTotal.With("error", err.Error(), "method", simulatorInfo, "data_store", redisStore).Add(1)
		return nil, err
	}

	return sim, nil
}

// AddSimulatorType is the PersistentStorage implementation to write or update the build info for available simulators
func (p *PersistentStorage) AddSimulatorType(sim *Simulator) error {
	defer func(now time.Time) {
		requestTime.With("method", addSimulatorType, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	err := p.setRedisJSON("HSET", simulatorKey, sim.Name, sim)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", addSimulatorType, "data_store", redisStore).Add(1)
		return err
	}

	return nil
}

// RemoveSimulatorType is the PersistentStorage implementation to delete a simulator type from storage
func (p *PersistentStorage) RemoveSimulatorType(simType string) error {
	defer func(now time.Time) {
		requestTime.With("method", removeSimulatorType, "data_store", redisStore).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	conn := p.pool.Get()
	defer conn.Close()

	_, err := conn.Do("HDEL", simulatorKey, simType)
	if err != nil {
		operationsErrorTotal.With("error", err.Error(), "method", removeSimulatorType, "data_store", redisStore).Add(1)
		return err
	}

	return nil
}

// SimulatorNames is the localStorage implementation to get all of the names of simulators we can start
func (l *localStorage) SimulatorNames() ([]string, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	if len(l.simulators) == 0 {
		return nil, ErrNoSimulator
	}

	resultNames := make([]string, len(l.simulators))

	i := 0
	for name := range l.simulators {
		resultNames[i] = name
		i++
	}

	return resultNames, nil
}

// SimulatorInfo is the localStorage implementation to get the details needed to run a specific server
func (l *localStorage) SimulatorInfo(simName string) (*Simulator, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	sim, ok := l.simulators[simName]
	if !ok {
		return nil, ErrNoSimulator
	}

	return sim, nil
}

// AddSimulatorType is the localStorage implementation to write or update the build info for available simulators
func (l *localStorage) AddSimulatorType(sim *Simulator) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	l.simulators[sim.Name] = sim

	return nil
}

// RemoveSimulatorType is the persistentStorage implementation to delete a simulator type from storage
func (l *localStorage) RemoveSimulatorType(simType string) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	delete(l.simulators, simType)

	return nil
}

// SimulatorNames is the localStorage implementation to get all of the names of simulators we can start
func (l *localSimHostStorage) SimulatorNames() ([]string, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	if len(l.simulators) == 0 {
		return nil, ErrNoSimulator
	}

	resultNames := make([]string, len(l.simulators))

	i := 0
	for name := range l.simulators {
		resultNames[i] = name
		i++
	}

	return resultNames, nil
}

// SimulatorInfo is the localStorage implementation to get the details needed to run a specific server
func (l *localSimHostStorage) SimulatorInfo(simName string) (*Simulator, error) {
	l.mu.Lock()
	defer l.mu.Unlock()

	sim, ok := l.simulators[simName]
	if !ok {
		return nil, ErrNoSimulator
	}

	return sim, nil
}

// AddSimulatorType is the localStorage implementation to write or update the build info for available simulators
func (l *localSimHostStorage) AddSimulatorType(sim *Simulator) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	l.simulators[sim.Name] = sim

	return nil
}

// RemoveSimulatorType is the persistentStorage implementation to delete a simulator type from storage
func (l *localSimHostStorage) RemoveSimulatorType(simType string) error {
	l.mu.Lock()
	defer l.mu.Unlock()

	delete(l.simulators, simType)

	return nil
}
