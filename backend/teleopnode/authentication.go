package teleopnode

import (
	"encoding/json"
	"log"
	"net/http"
	"time"

	"context"

	"fmt"

	jwt "github.com/dgrijalva/jwt-go"
	"github.com/gorilla/mux"
	"github.com/zippyai/zippy/backend/data"
	"github.com/zippyai/zippy/backend/services"
	"github.com/zippyai/zippy/backend/teleop"
)

type contextKey string

// String exposes a context keys value
func (c contextKey) String() string {
	return string(c)
}

const (
	jwtClaimsKey = contextKey("claims")
)

func (a *App) registerClient(w http.ResponseWriter, r *http.Request) {
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	defer func(now time.Time) {
		httpRequestTime.With("endpoint", pathTmpl, "requester", "N/A").Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	var authView *teleop.UserAuth
	err = json.NewDecoder(r.Body).Decode(&authView)
	if err != nil {
		log.Println("got error trying to decode new user register: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	newAuth := teleop.UserAuthFromView(authView)

	newUser, err := services.CreateUserAuth(a.storage, newAuth)
	if err != nil {
		log.Println("got error registering new user: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	log.Printf("new user with userName: %s was created\n", newUser.UserName)

	view := teleop.NewUserView(newUser)
	respondJSON(w, r, "", view)
}

func (a *App) loginClient(w http.ResponseWriter, r *http.Request) {
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	startTime := time.Now()
	var authView *teleop.UserAuth
	err = json.NewDecoder(r.Body).Decode(&authView)
	if err != nil {
		log.Println("got error trying to decode user auth: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	auth := teleop.UserAuthFromView(authView)

	user, err := services.AuthUser(a.storage, auth)
	if err != nil {
		log.Println("user not authorized to login: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusNotFound)
		return
	}

	log.Printf("User with userName: %s logged in\n", auth.UserName)

	cookie, err := services.CreateAuthCookie(user.UserID, a.authRealm, a.secrets.TokenSigningKey)
	if err != nil {
		log.Println("unable to create new cookie: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusInternalServerError)
		return
	}

	http.SetCookie(w, cookie)
	w.WriteHeader(http.StatusOK)
	httpRequestTime.With("endpoint", pathTmpl, "requester", user.UserID).Observe(time.Since(startTime).Seconds() * 1e3)
	httpRequestsTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", user.UserID, "code", "200").Add(1)
}

func (a *App) logoutClient(w http.ResponseWriter, r *http.Request) {
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

	deleteCookie := &http.Cookie{
		Name:   services.AuthKey,
		Value:  "",
		MaxAge: -1,
	}

	log.Println("User logged out")

	http.SetCookie(w, deleteCookie)
	w.WriteHeader(http.StatusOK)
	httpRequestsTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", requesterID, "code", "200").Add(1)
}

func (a *App) promoteUser(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	userID := vars["uid"]
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

	if err := services.IsAdmin(a.storage, requesterID); err != nil {
		log.Println("User is not an admin: ", err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusUnauthorized)
		return
	}

	if err := services.PromoteUser(a.storage, userID); err != nil {
		log.Printf("unable to promote user with id: %s error: %s", userID, err)
		http.Error(w, err.Error(), http.StatusBadRequest)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
		return
	}

	log.Printf("user with id: %s has been promoted to admin\n", requesterID)

	w.WriteHeader(http.StatusOK)
	httpRequestsTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", requesterID, "code", "200").Add(1)
}

func (a *App) generatePermissionlessToken(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	vehicleID := vars["vid"]
	route := mux.CurrentRoute(r)
	pathTmpl, err := route.GetPathTemplate()
	if err != nil {
		log.Println("unable to get route path from request")
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	defer func(now time.Time) {
		httpRequestTime.With("endpoint", pathTmpl, "requester", "N/A").Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	vehicle, err := services.CreateVehicle(a.storage, vehicleID)
	if err != nil {
		log.Println("unable to create new vehicle")
		respondError(w, r, err, "", pathTmpl, http.StatusInternalServerError)
		return
	}

	signedToken, err := services.CreateSignedKey(vehicle.VehicleID, a.authRealm, 0, a.secrets.TokenSigningKey)
	if err != nil {
		log.Println("unable to generate signed token: ", err)
		respondError(w, r, err, "", pathTmpl, http.StatusInternalServerError)
		return
	}

	if err = services.CreateVehicleAuth(a.storage, vehicle.VehicleID, signedToken, data.Inactive); err != nil {
		err = fmt.Errorf("unable to update the auth of the vehicle with ID: %s error: %s", vehicle.VehicleID, err)
		log.Println(err.Error())
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}

	log.Printf("vehicle with ID: %s had a new permissionless auth token generated for it\n", vehicle.VehicleID)

	respondJSON(w, r, "", teleop.VehicleAuthView{
		Token: signedToken,
	})
}

func (a *App) setTokenValidity(w http.ResponseWriter, r *http.Request) {
	vars := mux.Vars(r)
	vehicleID := vars["vid"]
	invalid := r.URL.Query().Get("invalidate")
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

	if err := services.IsAdmin(a.storage, requesterID); err != nil {
		log.Println("User is not an admin: ", err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusUnauthorized)
		return
	}

	validityState := data.Valid
	if invalid == "true" {
		validityState = data.InValid
	}

	if err := services.CreateVehicleAuth(a.storage, vehicleID, "", validityState); err != nil {
		err = fmt.Errorf("unable to update the validity of the vehicle with ID: %s error: %s", vehicleID, err)
		log.Println(err.Error())
		respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
		return
	}

	w.WriteHeader(http.StatusOK)
	httpRequestsTotal.With("endpoint", pathTmpl, "method", r.Method, "requester", requesterID, "code", "200").Add(1)
}

func (a *App) generateVehicleToken(w http.ResponseWriter, r *http.Request) {
	vehicleID := r.URL.Query().Get("vehicle_id")
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
		respondError(w, r, err, "", pathTmpl, http.StatusBadRequest)
		return
	}
	defer func(now time.Time) {
		httpRequestTime.With("endpoint", pathTmpl, "requester", requesterID).Observe(time.Since(now).Seconds() * 1e3)
	}(time.Now())

	if err := services.IsAdmin(a.storage, requesterID); err != nil {
		log.Println("User is not an admin: ", err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusUnauthorized)
		return
	}

	vehicle, err := services.CreateVehicle(a.storage, vehicleID)
	if err != nil {
		log.Println("unable to create new vehicle")
		respondError(w, r, err, requesterID, pathTmpl, http.StatusInternalServerError)
		return
	}

	signedToken, err := services.CreateSignedKey(vehicle.VehicleID, a.authRealm, 0, a.secrets.TokenSigningKey)
	if err != nil {
		log.Println("unable to generate signed token: ", err)
		respondError(w, r, err, requesterID, pathTmpl, http.StatusInternalServerError)
		return
	}

	if err = services.CreateVehicleAuth(a.storage, vehicle.VehicleID, signedToken, data.Valid); err != nil {
		err = fmt.Errorf("unable to update the auth of the vehicle with ID: %s error: %s", vehicle.VehicleID, err)
		log.Println(err.Error())
		respondError(w, r, err, requesterID, pathTmpl, http.StatusBadRequest)
		return
	}

	log.Printf("vehicle with ID: %s had a new auth token generated for it\n", vehicle.VehicleID)

	respondJSON(w, r, requesterID, teleop.VehicleAuthView{
		VehicleID: vehicle.VehicleID,
		Token:     signedToken,
	})
}

// middleware to make sure that a user is authorized to call this resource
func (a *App) validateToken(next http.HandlerFunc) http.HandlerFunc {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		token, err := services.ValidTokenFromRequest(r, a.secrets.TokenSigningKey)
		if err != nil {
			log.Println("unathorized user tried to access resource: ", err)
			respondError(w, r, err, "", "", http.StatusUnauthorized)
			return
		}

		claims, ok := token.Claims.(jwt.MapClaims)
		if !ok {
			err = fmt.Errorf("unable to parse the jwt claims from the token")
			respondError(w, r, err, "", "", http.StatusUnauthorized)
			return
		}

		ctx := context.WithValue(r.Context(), jwtClaimsKey, claims)
		r = r.WithContext(ctx)
		next(w, r)
	})
}

func respondJSON(w http.ResponseWriter, r *http.Request, requesterID string, payload interface{}) {
	jBlob, err := json.Marshal(payload)
	if err != nil {
		log.Println("unable to write response: ", err)
		respondError(w, r, err, requesterID, "", http.StatusInternalServerError)
		return
	}

	httpRequestsTotal.With("endpoint", "", "method", r.Method, "requester", requesterID, "code", "200").Add(1)
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	w.Write(jBlob)
}
