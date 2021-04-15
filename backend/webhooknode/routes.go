package webhooknode

import (
	"net/http"

	"github.com/gorilla/mux"
)

// NewWebhookRouter creates a router for all web hook endpoints
func NewWebhookRouter(m *WebhookManager) *mux.Router {
	router := mux.NewRouter()

	// postmates dispatch web hook
	router.HandleFunc("/postmates", m.postmatesDispatch)

	// this route is needed for the LB to check the health of the server
	router.HandleFunc("/health", albHealthCheck)

	return router
}

func albHealthCheck(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusOK)
}
