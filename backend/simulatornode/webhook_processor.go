package simulatornode

import (
	"bytes"
	"encoding/json"
	"errors"
	"log"
	"net/http"
)

var (
	// ErrPostFailed is returned if the statuscode gotten from the webhook post is not 200
	ErrPostFailed = errors.New("got a bad status code from post expecting 200")
)

type webhookSend struct {
	sendURL string
	payload interface{}
}

// WebhookSender is the interface to send a given payload to a destination
type webhookSender interface {
	start()
	send(url string, payload interface{})
}

type networkSender struct {
	client   http.Client
	sendChan chan *webhookSend
}

// NewNetworkSender creates a WebhookSender for network sends
func newNetworkSender() webhookSender {
	return &networkSender{
		client:   http.Client{},
		sendChan: make(chan *webhookSend, 5),
	}
}

// Start is the networkSender implementation to start a running loop to send webhook msgs
// This is a blocking loop and should be invoked in a go routine
func (n *networkSender) start() {
	for msg := range n.sendChan {
		go func(msg *webhookSend) {
			err := n.networkSend(msg)
			if err != nil {
				log.Println("error in sending webhook message: ", err)
				return
			}
		}(msg)
	}
}

// Send is the networkSender implementation to send a payload to a given url
func (n *networkSender) send(url string, payload interface{}) {
	errorChan := make(chan error)
	defer close(errorChan)
	msg := &webhookSend{
		sendURL: url,
		payload: payload,
	}

	n.sendChan <- msg
}

func (n *networkSender) networkSend(w *webhookSend) error {
	jBlob, err := json.Marshal(w.payload)
	if err != nil {
		return err
	}

	req, err := http.NewRequest("POST", w.sendURL, bytes.NewReader(jBlob))
	if err != nil {
		return err
	}

	resp, err := n.client.Do(req)
	if err != nil {
		return err
	}

	if resp.StatusCode != http.StatusOK {
		return ErrPostFailed
	}

	return nil

}
