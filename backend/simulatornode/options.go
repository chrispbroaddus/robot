package simulatornode

import (
	"fmt"

	"github.com/zippyai/zippy/backend/configs"
	"github.com/zippyai/zippy/backend/data"
)

// SimHostOptions is used to configure the sim host server
type SimHostOptions struct {
	configs *configs.Config
	secrets *configs.Secrets
	storage data.SimulatorHostStorage
	env     configs.Environment
}

// NewSimHostOptions create a new options for the sim host from config files
func NewSimHostOptions(conf *configs.Config, secrets *configs.Secrets) (*SimHostOptions, error) {
	opts := &SimHostOptions{
		configs: conf,
		secrets: secrets,
		env:     configs.Environment(conf.Environment),
	}

	storage, err := opts.newStorage()
	if err != nil {
		return nil, err
	}
	opts.storage = storage

	return opts, nil
}

func (o *SimHostOptions) newStorage() (data.SimulatorHostStorage, error) {
	switch o.env {
	case configs.Local:
		return data.NewSimHostLocalStorage(), nil
	case configs.Production:
		return data.NewPersistentStorage(o.rdsConnectionString(), o.configs.Redis.Server, o.secrets.Redis.Password, o.configs.S3.BucketName, o.configs.S3.Region, o.secrets.AWS.AccessKey, o.secrets.AWS.SecretKey, o.configs.Redis.DataBase)
	default:
		return data.NewSimHostLocalStorage(), nil
	}
}

// rdsConnectionString returns the connection string from it's values to be used by sql package
func (o *SimHostOptions) rdsConnectionString() string {
	return fmt.Sprintf("host=%s port=%d dbname=%s user=%s password=%s", o.configs.Database.Host, o.configs.Database.Port, o.configs.Database.DBName, o.secrets.Database.Username, o.secrets.Database.Password)
}
