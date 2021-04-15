package configs

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"strings"

	"github.com/zippyai/zippy/backend/data"
)

// Environment enum for the type of run this is letting us know how to set it up
type Environment int

const (
	// Testing is for a test env only should be used with routing
	Testing Environment = iota
	// Local is a localhost run
	Local
	// Staging is a staging run
	Staging
	// Production is a prod run
	Production
)

// UnmarshalJSON is the json unmarshaler implementation for Environment
func (e *Environment) UnmarshalJSON(b []byte) error {
	var s string
	if err := json.Unmarshal(b, &s); err != nil {
		return err
	}

	switch strings.ToLower(s) {
	case "local":
		*e = Local
	case "staging":
		*e = Staging
	case "production":
		*e = Production
	case "testing":
		*e = Testing
	default:
		return fmt.Errorf("unable to parse %s as a Environment", s)
	}

	return nil
}

// MarshalJSON is the json marshaler implementation for Environment
func (e Environment) MarshalJSON() ([]byte, error) {
	var s string
	switch e {
	case Local:
		s = "local"
	case Staging:
		s = "staging"
	case Production:
		s = "production"
	case Testing:
		s = "testing"
	default:
		return nil, fmt.Errorf("unable to parse %d as a string", int(e))
	}

	return json.Marshal(s)
}

// Config contains all of the volume mounted configurations needed for each deployment environment
type Config struct {
	// basic host information
	Environment Environment `json:"env,omitempty"`
	Port        int         `json:"port,omitempty"`
	DebugPort   int         `json:"debug_port,omitempty"`
	MetricsPort int         `json:"metrics_port,omitempty"`
	Domain      string      `json:"domain,omitempty"`
	// paths to the served up assets
	AssetPath      string `json:"asset_path,omitempty"`
	PlainAssetPath string `json:"plain_asset_path,omitempty"`
	// the data storage we'll be using this run
	Storage data.Storage `json:"-"`
	// the connection information used for the rds data store
	Database *Database `json:"data_base,omitempty"`
	// redis connection information
	Redis *Redis `json:"redis,omitempty"`
	// ssh config for simulator server
	SSHConfig *SSHConfig `json:"ssh_config,omitempty"`
	// aws s3 connection information
	S3 *AWSS3 `json:"aws_s3,omitempty"`
}

// Database contains Database connection information
type Database struct {
	DBName  string `json:"db_name"`
	Host    string `json:"host"`
	Port    int    `json:"port"`
	SSLMode string `json:"ssl_mode"`
}

// Redis contains the server and db number to connect to
type Redis struct {
	Server   string `json:"server"`
	DataBase int    `json:"data_base"`
}

// AWSS3 is the bucket and connection information for s3
type AWSS3 struct {
	BucketName string `json:"bucket"`
	Region     string `json:"region"`
}

// LoadConfig creates a config structure from a config file at the path passed in
func LoadConfig(filePath string) (*Config, error) {
	fBlob, err := ioutil.ReadFile(filePath)
	if err != nil {
		return nil, err
	}

	config := new(Config)
	if err = json.Unmarshal(fBlob, config); err != nil {
		return nil, err
	}

	return config, nil
}

// Secrets contains all of the secrets used by the app in a separate json file
type Secrets struct {
	TokenSigningKey []byte        `json:"token,omitempty"`
	Database        *DBSecrets    `json:"data_base,omitempty"`
	Redis           *RedisSecrets `json:"redis,omitempty"`
	AWS             *AWSSecrets   `json:"aws,omitempty"`
}

// DBSecrets contains the secrets related to connecting to an rds
type DBSecrets struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

// RedisSecrets contains the secrets used for connecting to redis
type RedisSecrets struct {
	Password string `json:"password"`
}

// SSHConfig contains the information to ssh onto the simulator server
type SSHConfig struct {
	Username      string `json:"username"`
	KeyFile       string `json:"key_file_path"`
	SimServerAddr string `json:"sim_server"`
}

// AWSSecrets contains the secrets used to connect to aws
type AWSSecrets struct {
	AccessKey string `json:"access_key"`
	SecretKey string `json:"secret_key"`
}

// LoadSecrets creates a secrets structure from a file that is passed in
func LoadSecrets(filePath string) (*Secrets, error) {
	sBlob, err := ioutil.ReadFile(filePath)
	if err != nil {
		return nil, err
	}

	secrets := new(Secrets)
	if err = json.Unmarshal(sBlob, secrets); err != nil {
		return nil, err
	}

	return secrets, nil
}
