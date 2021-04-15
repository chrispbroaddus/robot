package teleopnode

import (
	"context"
	"log"
	"net/http"
	"time"

	"github.com/gorilla/mux"
)

func (a *App) notificationWs(w http.ResponseWriter, r *http.Request) {
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	userID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer func(now time.Time) {
		websocketLifeSpan.With("endpoint", pathTmpl, "requester", userID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	log.Println("starting users notify websocket")
	defer log.Println("closing user notification websocket")

	// upgrade from ordinary http request to websocket
	clientConn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("error upgrading to websocket connection:", err)
		return
	}
	openWebsockets.With("endpoint", pathTmpl, "requester", userID).Add(1)
	defer openWebsockets.With("endpoint", pathTmpl, "requester", userID).Add(-1)

	// read from the connection
	fromOperator := make(chan []byte)
	errorChan := make(chan error)
	go func(errorChan chan error) {
		if err := readMessages(context.Background(), fromOperator, clientConn); err != nil {
			errorChan <- err
		}
	}(errorChan)

	listener := a.userNotifier.RegisterListener(userID)

OuterLoop:
	for {
		select {
		case msg := <-listener:
			err = clientConn.WriteJSON(msg)
			if err != nil {
				log.Println("error writing notification to user: ", err)
				break OuterLoop
			}
		case err = <-errorChan:
			log.Println("error reading from notify websocket with client: ", err)
			break OuterLoop
		case buf := <-fromOperator:
			if buf == nil {
				log.Println("empty message sent breaking out of notifier loop")
				break OuterLoop
			}
		}
	}
}
