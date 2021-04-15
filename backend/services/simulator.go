package services

import (
	"bufio"
	"context"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"net"
	"strings"
	"sync"

	"golang.org/x/crypto/ssh"

	"github.com/zippyai/zippy/backend/data"
)

var (
	// ErrSimulatorNotRunning is returned if you try and kill a simulator that is not currently running
	ErrSimulatorNotRunning = errors.New("the simulator you're trying to kill is no longer running")
)

// SimulatorController contains all of the interfaces needed to control the full lifecycle of a sim
type SimulatorController interface {
	SimulatorRunCloser
	RegisterNotifier
}

// SimulatorRunCloser is a mockable interface for running a new sim
type SimulatorRunCloser interface {
	Run(ctx context.Context, name string, simulator *data.Simulator) error
	Close(simulatorName string) error
}

type sshSimRunner struct {
	mu             sync.Mutex
	callbackDomain string
	userName       string
	keyFile        string
	simServer      string
	runningSims    map[string]*simulatorReporter
}

// simulatorReporter is the Notifier base for each simulator letting subscribers know if there is a status change on the sim
type simulatorReporter struct {
	ctx       context.Context
	cancel    context.CancelFunc
	errorChan chan interface{}
}

func (r *sshSimRunner) reporter(ctx context.Context, simulatorID string) *simulatorReporter {
	r.mu.Lock()
	defer r.mu.Unlock()

	reporter, ok := r.runningSims[simulatorID]
	if !ok {
		childCtx, cancelFunc := context.WithCancel(ctx)
		reporter = &simulatorReporter{
			ctx:       childCtx,
			cancel:    cancelFunc,
			errorChan: make(chan interface{}),
		}
	}
	r.runningSims[simulatorID] = reporter

	return reporter
}

// simulatorSession contains all of the connection and logging info for a concurant run of a simulator
type simulatorSession struct {
	conatinerID string
	logSession  *ssh.Session
	client      *ssh.Client
}

// Run is a real implementation used to start up a new simulator and keep alive until a kill command comes through
func (r *sshSimRunner) Run(ctx context.Context, name string, simulator *data.Simulator) error {
	// TODO super hack that needs to be replaced with hitting an endpoint to set this up
	// when we have a full deployment system for sims
	errChan := make(chan error)
	simSession := make(chan *simulatorSession)
	output := make(chan *bufio.Scanner)

	reporter := r.reporter(ctx, name)

	go func(errChan chan error, simSession chan *simulatorSession, output chan *bufio.Scanner) {
		client, err := createSSHClient(r.userName, r.keyFile, r.simServer)
		if err != nil {
			errChan <- err
			return
		}
		defer client.Close()

		session, err := client.NewSession()
		if err != nil {
			errChan <- err
			return
		}

		dockerCommand := `nvidia-docker run --rm -d -e "DISPLAY=:0" --volume=/tmp/.X11-unix:/tmp/.X11-unix:rw --volume=/usr/lib/x86_64-linux-gnu/libXv.so.1:/usr/lib/x86_64-linux-gnu/libXv.so.1  server1.zippy:5000/sim:master`
		startCommand := fmt.Sprintf("%s ./script/nvidia-docker-sim-runner.sh -n %s -f %s -d %s", dockerCommand, name, simulator.SimulatorConfigFile, r.callbackDomain)

		containerIDBlob, err := session.Output(startCommand)
		if err != nil {
			errChan <- err
			return
		}
		containerID := strings.TrimSpace(string(containerIDBlob))
		session.Close()

		logSession, err := client.NewSession()
		if err != nil {
			errChan <- err
			return
		}

		// setup pty so we can kill the command remotely
		mode := ssh.TerminalModes{
			ssh.TTY_OP_ISPEED: 14400,
			ssh.TTY_OP_OSPEED: 14400,
		}

		if err = logSession.RequestPty("xterm", 40, 80, mode); err != nil {
			errChan <- err
			return
		}

		outReader, err := logSession.StdoutPipe()
		if err != nil {
			errChan <- err
			return
		}

		errReader, err := logSession.StderrPipe()
		if err != nil {
			errChan <- err
			return
		}

		outputReader := io.MultiReader(outReader, errReader)
		outScanner := bufio.NewScanner(outputReader)
		output <- outScanner

		simSession <- &simulatorSession{
			client:      client,
			logSession:  logSession,
			conatinerID: containerID,
		}

		logCommand := fmt.Sprintf("docker logs -f %s", containerID)

		if err := logSession.Run(logCommand); err != nil {
			log.Println("long running log command exited with error: ", err)
		}
	}(errChan, simSession, output)

	// simulatorSession that is set and used concurrently
	var runningSession *simulatorSession
	for {
		select {
		case err := <-errChan:
			// if we get an error during the setup of the sim we need to clean everything up and report the error
			if runningSession != nil {
				err := runningSession.cleanUpSimulator()
				if err != nil {
					return fmt.Errorf("error cleaning up simulator after startup failed: %s", err)
				}
			}

			return err
		case session := <-simSession:
			// used to set the runningSession when it is ready to be killed
			runningSession = session
		case scanner := <-output:
			// used to capture log lines written by the tail of the sim container
			go func() {
				for scanner.Scan() {
					//log.Println(scanner.Text())
				}
			}()
		case <-reporter.ctx.Done():
			// kill the running simulator and clean it up if it exists
			if runningSession != nil {
				err := runningSession.cleanUpSimulator()
				if err != nil {
					return fmt.Errorf("error cleaning up simulator when told to die: %s", err)
				}
			}

			return nil
		}
	}
}

func (s *simulatorSession) cleanUpSimulator() error {
	killSession, err := s.client.NewSession()
	if err != nil {
		return err
	}

	killCommand := fmt.Sprintf("docker kill %s", s.conatinerID)

	err = killSession.Start(killCommand)
	if err != nil {
		fmt.Println("error in running kill command: ", err)
		return err
	}

	err = s.logSession.Close()
	if err != nil {
		return err
	}

	return nil
}

// Close is a real implementation used to clean up a running simulator
func (r *sshSimRunner) Close(simulatorName string) error {
	r.mu.Lock()
	defer r.mu.Unlock()

	reporter, ok := r.runningSims[simulatorName]
	if !ok {
		return ErrSimulatorNotRunning
	}

	reporter.cancel()
	delete(r.runningSims, simulatorName)
	return nil
}

func (r *sshSimRunner) RegisterListener(id string) chan interface{} {
	r.mu.Lock()
	defer r.mu.Unlock()

	reporter, ok := r.runningSims[id]
	if !ok {
		reporter = &simulatorReporter{}
	}

	if reporter.errorChan == nil {
		reporter.errorChan = make(chan interface{})
	}
	r.runningSims[id] = reporter

	return reporter.errorChan
}

func (r *sshSimRunner) Notify(id string, msg interface{}) error {
	r.mu.Lock()
	defer r.mu.Unlock()

	reporter, ok := r.runningSims[id]
	if !ok {
		return ErrNoListener
	}

	if reporter.errorChan == nil {
		return fmt.Errorf("reporters error channel is nil")
	}

	go func() {
		reporter.errorChan <- msg
	}()

	return nil
}

// SetUpSimulatorRunner creates a real implementation of the SimulatorRunCloser interface
func SetUpSimulatorRunner(callbackDomain, userName, keyFile, simServer string) SimulatorController {
	return &sshSimRunner{
		callbackDomain: callbackDomain,
		userName:       userName,
		keyFile:        keyFile,
		simServer:      simServer,
		runningSims:    make(map[string]*simulatorReporter),
	}
}

// GetAllSimulatorTypes gets all of the names of the simulators available
func GetAllSimulatorTypes(store data.SimulatorCache) ([]string, error) {
	return store.SimulatorNames()
}

// GetSimulator returns the connection information for a names simulator
func GetSimulator(store data.SimulatorCache, simulatorName string) (*data.Simulator, error) {
	return store.SimulatorInfo(simulatorName)
}

// UpdateSimulator sets a simulator in storage with the information provided
func UpdateSimulator(store data.SimulatorCache, name, file string) error {
	sim := &data.Simulator{
		Name:                name,
		SimulatorConfigFile: file,
	}

	return store.AddSimulatorType(sim)
}

// DeleteSimulatorType is used for removing a defined simulator type
func DeleteSimulatorType(store data.SimulatorCache, simulatorID string) error {
	return store.RemoveSimulatorType(simulatorID)
}

// StartSimulator starts a simulator on aws with the provided information
func StartSimulator(ctx context.Context, instanceName string, simRunner SimulatorController, simulator *data.Simulator) error {
	go func() {
		err := simRunner.Run(ctx, instanceName, simulator)
		if err != nil {
			log.Println("error starting up the simulator asynchronously: ", err)
			simRunner.Notify(instanceName, err)
		}
	}()

	return nil
}

// StopSimulator is used to call Close on the interface to kill the named running sim
func StopSimulator(simCloser SimulatorRunCloser, simulatorName string) error {
	return simCloser.Close(simulatorName)
}

func createSSHClient(userName, keyFile, simServer string) (*ssh.Client, error) {
	key, err := ioutil.ReadFile(keyFile)
	if err != nil {
		return nil, err
	}

	signer, err := ssh.ParsePrivateKey(key)
	if err != nil {
		return nil, err
	}

	config := &ssh.ClientConfig{
		User: userName,
		Auth: []ssh.AuthMethod{
			ssh.PublicKeys(signer),
		},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
	}

	return ssh.Dial("tcp", fmt.Sprintf("%s:%d", simServer, 22), config)
}

// GetPrivateIP returns a private ip for the vpn network that can be reached by the sim server
func GetPrivateIP() (string, error) {
	interfaces, err := net.Interfaces()
	if err != nil {
		return "", err
	}

	localhost := net.ParseIP("127.0.0.1")

	for _, i := range interfaces {
		addrs, err := i.Addrs()
		if err != nil {
			return "", err
		}

		for _, address := range addrs {
			switch v := address.(type) {
			case *net.IPNet:
				privateIP := v.IP
				v4 := v.IP.To4()
				if !privateIP.Equal(localhost) && v4 != nil {
					return privateIP.String(), nil
				}

			default:
				continue
			}
		}
	}

	return "", errors.New("failed to find private ip")
}
