package teleopnode

import (
	"log"
	"net/http"
	"time"

	"encoding/json"

	"github.com/gorilla/mux"
	"github.com/zippyai/zippy/backend/data"
	"github.com/zippyai/zippy/backend/services"
	"github.com/zippyai/zippy/backend/teleop"
)

func (a *App) requestControl(w http.ResponseWriter, r *http.Request) {
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
		log.Println("unable to get token claims from request error: ", err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
		return
	}
	defer func(now time.Time) {
		httpRequestTime.With("endpoint", pathTmpl, "requester", requesterID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	err = services.PromoteToController(a.storage, sessionID, requesterID)
	if err != nil {
		log.Println("unable to set user as as controller of vehicle: ", err)
		if err == data.ErrNoVehicle {
			respondError(w, r, err, requesterID, pathTmpl, http.StatusNotFound)
		} else {
			respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
		}
		return
	}

	log.Printf("user with id: %s is now in control of the vehicle", requesterID)
	w.WriteHeader(http.StatusOK)
	httpRequestsTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", requesterID, "code", "200").Add(1)
}

func (a *App) relinquishControl(w http.ResponseWriter, r *http.Request) {
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
		httpRequestTime.With("endpoint", pathTmpl, "requester", requesterID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	if err := services.RemoveController(a.storage, sessionID); err != nil {
		if err == data.ErrNoVehicle {
			respondError(w, r, err, requesterID, pathTmpl, http.StatusNotFound)
		} else {
			respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
		}
		return
	}

	log.Printf("vehicle: %s is no longer being controlled", sessionID)
	w.WriteHeader(http.StatusOK)
	httpRequestsTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", requesterID, "code", "200").Add(1)
}

func (a *App) authVehicleSession(w http.ResponseWriter, r *http.Request) {
	startTime := time.Now()
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	vehicleAuth := new(teleop.VehicleAuthView)
	if err := json.NewDecoder(r.Body).Decode(vehicleAuth); err != nil {
		log.Println("auth body was not able to be parsed for vehicle: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	r.Body.Close()

	if err := services.ValidVehicleAuth(a.storage, vehicleAuth); err != nil {
		log.Printf("unable to authenticate a vehicle with id: %s error: %s", vehicleAuth.VehicleID, err)
		respondError(w, r, err, "", pathTmpl, http.StatusUnauthorized)
		return
	}

	sessionCookie, err := services.CreateNewVehicleSession(a.storage, vehicleAuth.VehicleID)
	if err != nil {
		log.Printf("unable to save new session id for vehicle with id: %s error: %s", vehicleAuth.VehicleID, err)
		respondError(w, r, err, "", pathTmpl, http.StatusInternalServerError)
		return
	}

	log.Println("new vehicle session has been authorized for vehicleID: ", vehicleAuth.VehicleID)

	http.SetCookie(w, sessionCookie)
	w.WriteHeader(http.StatusOK)
	httpRequestTime.With("endpoint", pathTmpl, "requester", sessionCookie.Value).Observe(time.Since(startTime).Seconds() * 1e3)
}

// middleware to make sure that the requester is the one currently in controll of the vehicle
func (a *App) ensureControllingVehicle(next http.HandlerFunc) http.HandlerFunc {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		vars := mux.Vars(r)
		vehicleID := vars["sid"]
		route := mux.CurrentRoute(r)
		pathTmpl, err := route.GetPathTemplate()
		if err != nil {
			log.Println("unable to get route path from request")
			respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
			return
		}

		userID, err := userIDFromRequest(r)
		if err != nil {
			log.Println("unable to get userID from request error: ", err)
			respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
			return
		}

		// make sure that the user is either an admin or is in control of the vehicle currently
		if err := services.IsAdmin(a.storage, userID); err != nil {
			if err := services.UserIsInControl(a.storage, vehicleID, userID); err != nil {
				log.Printf("user with id: %s is not in control of the vehicle and can't do this error: %s\n", userID, err)
				respondError(w, r, err, userID, pathTmpl, http.StatusForbidden)
				return
			}
		}

		next(w, r)
	})
}
