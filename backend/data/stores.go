package data

import (
	"database/sql"
	"errors"
	"sync"
	"time"

	"github.com/aws/aws-sdk-go/aws"
	"github.com/aws/aws-sdk-go/aws/credentials"
	"github.com/aws/aws-sdk-go/aws/session"
	"github.com/aws/aws-sdk-go/service/s3/s3manager"
	"github.com/garyburd/redigo/redis"
	// "github.com/bmizerany/pq" is the postgres sql driver needed to open the connection
	_ "github.com/bmizerany/pq"
	uuid "github.com/satori/go.uuid"
)

var (
	// ErrNoUser returned when a user is not found for id
	ErrNoUser = errors.New("there is no user for that id")
	// ErrNoVehicle returned when a vehicle is not found for an id
	ErrNoVehicle = errors.New("there is no vehicle for that id")
	// ErrNoCamera returned when a vehicle does not have a camera for that id
	ErrNoCamera = errors.New("there is no camera on the vehicle with that id")
	// ErrBadTimeStamp returned when a timestamp that is provided is zero
	ErrBadTimeStamp = errors.New("the timestamp provided is zero")
)

const (
	// time out used on a connection before we purge it and bring it back into the pool
	redisConnTimeOut = time.Minute
)

// Storage is the composed interface for all data storage interfaces
type Storage interface {
	UserStorage
	VehicleStorage
	SimulatorCache
}

// UserStorage used for the different data stores dealing with users
type UserStorage interface {
	FindUser(string) (*User, error)
	FindUserByName(string) (*User, error)
	UserAuthentication
}

// UserAuthentication used for creating, and updating user auth states
type UserAuthentication interface {
	CreateUserAuth(userID, userName, password string) (*User, error)
	UpdateUserAuth(userID, userName, password string) (*User, error)
	SetAdmin(userID string, isAdmin bool) error
}

// VehicleStorage used for the different data stores dealing with vehicles
type VehicleStorage interface {
	AuthVehicle(vehicleID, token string) (TokenState, error)
	CreateVehicle(vehicleID string) (*Vehicle, error)
	CreateVehicleAuth(vehicleID, token string, tokenState TokenState) error
	UpdateVehicleAuth(vehicleID, token string, tokenState TokenState) error
	SetVehicleLocation(vehicleID string, location *Location) error
	SaveCameraFrame(vehicleID string, frame *CameraSample) error
	SetVehicleCameras(vehicleID string, cameraIDs ...string) error
	FindVehicle(string) (*Vehicle, error)
	VehicleCache
}

// VehicleCache used for working with the users interactions with a vehicles cache
type VehicleCache interface {
	// Vehicle specific information
	GetVehicleCache(sessionID string) (*VehicleSession, error)
	SetVehicleCache(sessionID string, vehicle *VehicleSession) error
	// vehicle's camera specific information
	GetVehicleCameraCache(vehicleID, cameraID string) (*Camera, error)
	GetVehicleCamerasCache(vehicleID string) (map[string]*Camera, error)
	SetVehicleCameraCache(vehicleID string, camera *Camera) error
}

// SimulatorCache used to work with deployable simulators
type SimulatorCache interface {
	SimulatorNames() ([]string, error)
	SimulatorInfo(simName string) (*Simulator, error)
	AddSimulatorType(sim *Simulator) error
	RemoveSimulatorType(simType string) error
}

// SimulatorHostStorage is the top level interface for simulator host storage ops
type SimulatorHostStorage interface {
	SimulatorCache
	SimulatorSessions
	SimulatorRecipeBooks
}

// SimulatorRecipeBooks is used to manage recipe books
type SimulatorRecipeBooks interface {
	CreateRecipeBook(r *SimulatorRecipeBook) error
	RemoveRecipeBook(recipeBookID string) error
	RemoveRecipe(recipeBookID, recipeID string) error
	AddRecipe(recipeBookID, version string, r *SimulatorRecipe) error
	FindRecipeBook(bookID string) (*SimulatorRecipeBook, error)
}

// SimulatorSessions is used to manage simulator sessions in storage
type SimulatorSessions interface {
	SetSimSession(s *SimulatorSession) error
	UpdateSimSessionState(simID string, state SimulatorState) error
	FindSimSession(simID string) (*SimulatorSession, error)
	RemoveSimSession(simID string) error
}

type localStorage struct {
	mu         sync.Mutex
	users      map[string]*User
	vehicles   map[string]*Vehicle
	simulators map[string]*Simulator
	// vehicleSessions is keyed to simulators name
	vehicleSessions map[string]*VehicleSession
	// local storage dir used to serve up saved frames
	imageDir string
}

// NewLocalStorage creates a localstorage object and adds the first user to it
func NewLocalStorage(imageDir string) Storage {
	storage := &localStorage{
		users:           make(map[string]*User),
		vehicles:        make(map[string]*Vehicle),
		simulators:      make(map[string]*Simulator),
		vehicleSessions: make(map[string]*VehicleSession),
		imageDir:        imageDir,
	}

	baseAdmin := &User{
		UserID: uuid.NewV4().String(),
		Admin:  true,
		UserAuth: UserAuth{
			UserName: "admin",
			Password: "admin",
		},
	}

	// set up an initial vehicle with hardcoded id for testing the ui
	baseVehicle := NewVehicle("r01")

	// set up the initial hosted simulator
	baseSim := &Simulator{
		Name:                "default sim",
		SimulatorConfigFile: "/zippy/packages/unity_simulator/ZippySimUnity/Assets/StreamingAssets/simulator_settings.json",
	}
	storage.AddSimulatorType(baseSim)

	storage.users[baseAdmin.UserID] = baseAdmin
	storage.vehicles[baseVehicle.VehicleID] = baseVehicle
	return storage
}

type localSimHostStorage struct {
	mu          sync.Mutex
	simulators  map[string]*Simulator
	simSessions map[string]*SimulatorSession
	recipeBooks map[string]*SimulatorRecipeBook
	recipes     map[string]*SimulatorRecipe
}

// NewSimHostLocalStorage creates a local storage implementation for a SimulatorHostStorage
func NewSimHostLocalStorage() SimulatorHostStorage {
	storage := &localSimHostStorage{
		simulators:  make(map[string]*Simulator),
		simSessions: make(map[string]*SimulatorSession),
		recipeBooks: make(map[string]*SimulatorRecipeBook),
		recipes:     make(map[string]*SimulatorRecipe),
	}

	// set up the initial hosted simulator
	baseSim := &Simulator{
		Name:                "default sim",
		SimulatorConfigFile: "/zippy/packages/unity_simulator/ZippySimUnity/Assets/StreamingAssets/simulator_settings.json",
	}
	storage.AddSimulatorType(baseSim)
	return storage
}

// PersistentStorage is used by teleop and the sim host to connect to the postgres db, redis, and s3
type PersistentStorage struct {
	db   *sql.DB
	pool *redis.Pool
	s3   *s3Client
}

// NewPersistentStorage returns a Storage interface for rds and redis storage methods
func NewPersistentStorage(rdsConnection, redisServer, redisPass, bucketName, region, accessKey, secretKey string, redisDB int) (*PersistentStorage, error) {
	db, err := createDB(rdsConnection)
	if err != nil {
		return nil, err
	}

	client, err := createS3Client(bucketName, region, accessKey, secretKey)
	if err != nil {
		return nil, err
	}

	storage := &PersistentStorage{
		db:   db,
		pool: createPool(redisServer, redisPass, redisDB),
		s3:   client,
	}

	// set up the initial hosted simulator
	baseSim := &Simulator{
		Name:                "default sim",
		SimulatorConfigFile: "/zippy/packages/unity_simulator/ZippySimUnity/Assets/StreamingAssets/simulator_settings.json",
	}

	if err := storage.AddSimulatorType(baseSim); err != nil {
		return nil, err
	}

	return storage, nil
}

func createDB(connection string) (*sql.DB, error) {
	db, err := sql.Open("postgres", connection)
	if err != nil {
		return nil, err
	}

	if err = db.Ping(); err != nil {
		return nil, err
	}

	return db, nil
}

type s3Client struct {
	s3         *s3manager.Uploader
	bucketName string
}

func createS3Client(bucketName, region, accessKey, secretKey string) (*s3Client, error) {
	sess, err := session.NewSession(&aws.Config{
		Region:      aws.String(region),
		Credentials: credentials.NewStaticCredentials(accessKey, secretKey, ""),
	})

	if err != nil {
		return nil, err
	}

	s3client := s3manager.NewUploader(sess)

	return &s3Client{
		s3:         s3client,
		bucketName: bucketName,
	}, nil
}

func createPool(server, password string, db int) *redis.Pool {
	return &redis.Pool{
		MaxIdle:     80,
		MaxActive:   1000,
		IdleTimeout: time.Duration(10 * time.Second),
		Dial: func() (redis.Conn, error) {
			c, err := redis.Dial("tcp", server)
			if err != nil {
				return nil, err
			}

			if password != "" {
				if _, err := c.Do("AUTH", password); err != nil {
					c.Close()
					return nil, err
				}
			}

			if _, err := c.Do("SELECT", db); err != nil {
				c.Close()
				return nil, err
			}

			return c, nil
		},
		TestOnBorrow: func(c redis.Conn, t time.Time) error {
			if time.Since(t) > redisConnTimeOut {
				return nil
			}

			if _, err := c.Do("PING"); err != nil {
				return err
			}

			return nil
		},
	}
}
