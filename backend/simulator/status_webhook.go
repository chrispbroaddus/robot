package simulator

// StatusView contains a status update on the run status of the simulator session
type StatusView struct {
	SessionID    string `json:"id"`
	Status       string `json:"status"`
	ErrorMessage string `json:"error_msg,omitempty"`
}

// NewStatusMsg returns a StatusView msg
func NewStatusMsg(sessionID, status, errorMsg string) *StatusView {
	return &StatusView{
		SessionID:    sessionID,
		Status:       status,
		ErrorMessage: errorMsg,
	}
}
