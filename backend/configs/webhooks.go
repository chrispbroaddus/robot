package configs

import (
	"encoding/json"
	"io/ioutil"
	"os"
)

const (
	slackWebHookENV = "SLACK_POSTMATES"
)

// WebHookServerConfig config file to define webhook endpoints and the server
type WebHookServerConfig struct {
	Port              int    `json:"port"`
	SlackPostmatesURL string `json:"slack_postmates_url,omitempty"`
}

// LoadWebHookServerConfig creates a config structure from a config file
func LoadWebHookServerConfig(file string) (*WebHookServerConfig, error) {
	fBlob, err := ioutil.ReadFile(file)
	if err != nil {
		return nil, err
	}

	config := new(WebHookServerConfig)
	err = json.Unmarshal(fBlob, config)
	if err != nil {
		return nil, err
	}

	config.SlackPostmatesURL = os.Getenv(slackWebHookENV)

	return config, nil
}
