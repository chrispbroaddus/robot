package teleopnode

import (
	"fmt"

	"github.com/zippyai/zippy/backend/configs"
	"github.com/zippyai/zippy/backend/data"
)

const (
	localImageDir = "/tmp/images"
)

// Options contains the config and secrets and sets up everything that is calculable from the two
type Options struct {
	configs   *configs.Config
	secrets   *configs.Secrets
	storage   data.Storage
	authRealm string
	assets    Assets
	env       configs.Environment
}

// NewOptions creates a new Options structure setting up all of the options tha need config and secret data
func NewOptions(conf *configs.Config, secrets *configs.Secrets) (*Options, error) {
	opts := &Options{
		configs:   conf,
		secrets:   secrets,
		env:       configs.Environment(conf.Environment),
		authRealm: conf.Domain,
		assets:    generateAssets(conf.AssetPath, conf.PlainAssetPath),
	}

	storage, err := opts.newStorage()
	if err != nil {
		return nil, err
	}
	opts.storage = storage

	return opts, nil
}

// newStorage create a localStorage when we have a staging and prod version we'll add setup here
func (o *Options) newStorage() (data.Storage, error) {
	switch o.env {
	case configs.Local:
		return data.NewLocalStorage(localImageDir), nil
	case configs.Production:
		return data.NewPersistentStorage(o.rdsConnectionString(), o.configs.Redis.Server, o.secrets.Redis.Password, o.configs.S3.BucketName, o.configs.S3.Region, o.secrets.AWS.AccessKey, o.secrets.AWS.SecretKey, o.configs.Redis.DataBase)
	default:
		return data.NewLocalStorage(localImageDir), nil
	}
}

// rdsConnectionString returns the connection string from it's values to be used by sql package
func (o *Options) rdsConnectionString() string {
	return fmt.Sprintf("host=%s port=%d dbname=%s user=%s password=%s", o.configs.Database.Host, o.configs.Database.Port, o.configs.Database.DBName, o.secrets.Database.Username, o.secrets.Database.Password)
}
