package webhooknode

import (
	"bytes"
	"encoding/json"
	"io"
	"log"
	"net/http"

	"github.com/zippyai/zippy/backend/configs"
	"github.com/zippyai/zippy/backend/webhooks"
)

// WebhookManager contains all of the information to send and receive webhook messages
type WebhookManager struct {
	httpClient http.Client
	config     *configs.WebHookServerConfig
}

// NewWebhookManager creates a new manager for all of the web hook endpoints
func NewWebhookManager(config *configs.WebHookServerConfig) *WebhookManager {
	return &WebhookManager{
		httpClient: http.Client{},
		config:     config,
	}
}

func (m *WebhookManager) postmatesDispatch(w http.ResponseWriter, r *http.Request) {
	payload := new(webhooks.PostmatesDispatch)
	err := json.NewDecoder(r.Body).Decode(payload)
	if err != nil {
		log.Println("unable to unmarshal json payload: ", err)
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	slackMsg, err := webhooks.CreatePostmatesSlackPayload(payload)
	if err != nil {
		log.Println("unable to generate slack message payload: ", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	body, err := makeJSONBody(slackMsg)
	if err != nil {
		log.Println("unable to marshal json payload for slack message: ", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	_, err = m.httpClient.Post(m.config.SlackPostmatesURL, "application/json", body)
	if err != nil {
		log.Println("unable to POST json payload of slack message: ", err)
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
}

func makeJSONBody(payload interface{}) (io.Reader, error) {
	blob, err := json.Marshal(payload)
	if err != nil {
		return nil, err
	}

	return bytes.NewReader(blob), nil
}
