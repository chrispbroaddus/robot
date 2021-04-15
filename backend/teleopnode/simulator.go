package teleopnode

import (
	"context"
	"encoding/json"
	"log"
	"net/http"
	"time"

	"github.com/gorilla/mux"

	"github.com/zippyai/zippy/backend/services"
	"github.com/zippyai/zippy/backend/teleop"
)

func (a *App) getSimTypes(w http.ResponseWriter, r *http.Request) {
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	requesterID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer httpRequestTime.With("endpoint", pathTmpl, "requester", requesterID).Observe(time.Since(time.Now()).Seconds() * 1e3)

	names, err := services.GetAllSimulatorTypes(a.storage)
	if err != nil {
		log.Println("error in getting the types of simulators available: ", err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusInternalServerError)
		return
	}

	respondJSON(w, r, requesterID, names)
}

func (a *App) startSimulator(w http.ResponseWriter, r *http.Request) {
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	requesterID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer httpRequestTime.With("endpoint", pathTmpl, "requester", requesterID).Observe(time.Since(time.Now()).Seconds() * 1e3)

	defer r.Body.Close()

	simInfo := new(teleop.StartSimulator)
	err = json.NewDecoder(r.Body).Decode(simInfo)
	if err != nil {
		log.Println("error in parsing the start simulator input: ", err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
		return
	}

	simulator, err := services.GetSimulator(a.storage, simInfo.SimulatorType)
	if err != nil {
		log.Println("error in simulator from storage: ", err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
		return
	}

	ctx := context.Background()
	err = services.StartSimulator(ctx, simInfo.VehicleName, a.simController, simulator)
	if err != nil {
		log.Println("unable to start remote simulator: ", err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	httpRequestsTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", requesterID, "code", "200").Add(1)
}

func (a *App) killSimulator(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	sessionID := vars["sid"]
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	requesterID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer httpRequestTime.With("endpoint", pathTmpl, "requester", requesterID).Observe(time.Since(time.Now()).Seconds() * 1e3)

	if err := services.StopSimulator(a.simController, sessionID); err != nil {
		log.Println("unable to stop running simulator: ", err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
		return
	}

	w.WriteHeader(http.StatusOK)
	httpRequestsTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", requesterID, "code", "200").Add(1)
}

func (a *App) createSimulator(w http.ResponseWriter, r *http.Request) {
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	requesterID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer httpRequestTime.With("endpoint", pathTmpl, "requester", requesterID).Observe(time.Since(time.Now()).Seconds() * 1e3)

	simInfo := new(teleop.NewSimulator)
	err = json.NewDecoder(r.Body).Decode(simInfo)
	if err != nil {
		log.Println("unable to parse simulator information: ", err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
		return
	}

	err = services.UpdateSimulator(a.storage, simInfo.Name, simInfo.ConfigFilePath)
	if err != nil {
		log.Println("unable to update simulator in storage: ", err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	httpRequestsTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", requesterID, "code", "200").Add(1)
}

func (a *App) deleteSimulatorType(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	simulatorID := vars["sim_id"]
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	err = services.DeleteSimulatorType(a.storage, simulatorID)
	if err != nil {
		log.Printf("unable delete simulator type: %s from storage\n", simulatorID)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	w.WriteHeader(http.StatusOK)
}

func (a *App) simStatusSocket(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	sessionID := vars["sid"]
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	requesterID, err := userIDFromRequest(r)
	if err != nil {
		log.Println("unable to get userID from request: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer func(now time.Time) {
		websocketLifeSpan.With("endpoint", pathTmpl, "requester", requesterID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	log.Println("at listening for simulator to start")
	defer log.Println("leaving listening for simulator to start")

	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		log.Println("error upgrading to websocket connection:", err)
		return
	}
	defer conn.Close()
	openWebsockets.With("endpoint", pathTmpl, "requester", requesterID).Add(1)
	defer openWebsockets.With("endpoint", pathTmpl, "requester", requesterID).Add(-1)

	listener := a.simController.RegisterListener(sessionID)
	timeout := time.After(10 * time.Minute)
	reminder := time.NewTicker(10 * time.Second)
	defer reminder.Stop()
	conn.WriteJSON(&teleop.SimulatorEvent{
		ID:        sessionID,
		Status:    teleop.Start,
		TimeStamp: time.Now().UTC(),
		Message:   "Listening for simulator's start...",
	})
	websocketFramesSent.With("endpoint", pathTmpl, "requester", requesterID).Add(1)

	// read from the connection
	fromVehicle := make(chan []byte)
	errorChan := make(chan error)
	go func(errorChan chan error) {
		if err := readMessages(context.Background(), fromVehicle, conn); err != nil {
			errorChan <- err
		}
	}(errorChan)

	// these let the testing library know that it's safe to read from the websocket
	done()
	defer done()

OuterLoop:
	for {
		select {
		case msg := <-listener:
			defer websocketFramesSent.With("endpoint", pathTmpl, "requester", requesterID).Add(1)
			if msg != nil {
				log.Printf("simulator with id: %s failed to start with message: %v", sessionID, msg)
				failMsg := &teleop.SimulatorEvent{
					ID:        sessionID,
					Status:    teleop.Error,
					TimeStamp: time.Now().UTC(),
					Message:   msg.(error).Error(),
				}

				err = conn.WriteJSON(failMsg)
				if err != nil {
					log.Println("error writing json to client on failure of sim to start: ", err)
				}

				return
			}

			log.Printf("simulator with id: %s just started up\n", sessionID)
			successMsg := &teleop.SimulatorEvent{
				ID:        sessionID,
				Status:    teleop.Ready,
				TimeStamp: time.Now().UTC(),
				Message:   "work?",
			}

			err = conn.WriteJSON(successMsg)
			if err != nil {
				log.Println("failed")
				log.Println("error in writing to websocket: ", err)
			}

			return

		case <-timeout:
			log.Println("we timed out waiting for simulator to start")
			return
		case <-reminder.C:
			stayAliveMsg := &teleop.SimulatorEvent{
				ID:        sessionID,
				Status:    teleop.Ping,
				TimeStamp: time.Now().UTC(),
				Message:   "stay alive",
			}
			websocketFramesSent.With("endpoint", pathTmpl, "requester", requesterID).Add(1)

			err = conn.WriteJSON(stayAliveMsg)
			if err != nil {
				log.Println("error in writing stay alive to websocket: ", err)
				break OuterLoop
			}
		case err = <-errorChan:
			log.Println("error reading from notify sim start ws exiting now: ", err)
			break OuterLoop

		}
	}
}
