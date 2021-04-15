package teleopnode

import (
	"fmt"
	"html/template"
	"io/ioutil"
	"log"
	"net/http"
	"net/http/pprof"

	"github.com/gorilla/mux"
	"github.com/zippyai/zippy/backend/configs"
	"github.com/zippyai/zippy/backend/services"
)

const (
	loginPage = `<!doctype html><html><head>
	<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js"></script>
	<script src="/plain/static/login.js"></script>
	</head>
	<form method="POST" name="loginform" id="loginform">
		<input type="text" name="userName"/>
		<input type="password" name="password"/>
		<input type="submit"/>
	</form>
	<a href="register">register</a>
	</html>`

	registerPage = `<!doctype html><html><head>
	<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js"></script>
	<script src="/plain/static/register.js"></script>
	</head>
	<form method="POST" name="registerform" id="registerform">
		<input type="text" name="userName"/>
		<input type="password" name="password"/>
		<input type="submit"/>
	</form>
	</html>`
)

func (a *App) newAPIRouter(mainRouter *mux.Router) *mux.Router {
	r := mainRouter.PathPrefix("/api/v1").Subrouter()

	// handler to logout user
	r.HandleFunc("/logout", a.validateToken(a.logoutClient)).Methods("GET")

	// handler to promote user with userID uid to an admin
	r.HandleFunc("/user/{uid}/promote", a.validateToken(a.promoteUser)).Methods("POST")

	// handler to generate a vehicles auth token
	r.HandleFunc("/vehicle/generate", a.validateToken(a.generateVehicleToken)).Methods("POST")

	// handler to validate or invalidate vehicle auth tokens
	r.HandleFunc("/vehicle/token/{vid}/validate", a.validateToken(a.setTokenValidity)).Methods("PUT")

	// handler to list the connected robots
	r.HandleFunc("/vehicles", a.validateToken(a.handleListVehicles)).Methods("GET")

	// handler to list the connected robots
	r.HandleFunc("/vehicle/{sid}", a.validateToken(a.handleVehicle)).Methods("GET")

	r.HandleFunc("/ws/vehicle/{sid}/user/{clid}/subscribe", a.validateToken(a.handleSignalWebrtc))

	// handler to list the connected robots
	r.HandleFunc("/ws/vehicle/{sid}/location/subscribe", a.validateToken(a.handleSubscribeLocation))

	// handler to list the connected robots
	r.HandleFunc("/ws/vehicle/{sid}/camera/{cam}/subscribe", a.validateToken(a.handleSubscribeCamera))

	// handler for subscribing to everything from the vehicle
	r.HandleFunc("/ws/vehicle/{sid}/subscribe", a.validateToken(a.commandVehicle))

	// remove control of a vehicle
	r.HandleFunc("/vehicle/{sid}/relinquish-control", a.validateToken(a.ensureControllingVehicle(a.relinquishControl))).Methods("GET")

	// take over control of a vehicle
	r.HandleFunc("/vehicle/{sid}/request-control", a.validateToken(a.requestControl)).Methods("GET")

	// get the last frame for a vehicle and camera that the backend has
	r.HandleFunc("/vehicle/{vid}/camera/{cid}", a.validateToken(a.getFrames)).Methods("GET")

	r.HandleFunc("/simulators", a.validateToken(a.getSimTypes)).Methods("GET")
	r.HandleFunc("/simulator/start", a.validateToken(a.startSimulator)).Methods("POST")
	r.HandleFunc("/simulator/{sid}/kill", a.validateToken(a.killSimulator)).Methods("PUT")
	r.HandleFunc("/simulator/create", a.validateToken(a.createSimulator)).Methods("POST")
	r.HandleFunc("/simulator/{sim_id}/delete", a.validateToken(a.deleteSimulatorType)).Methods("DELETE")
	r.HandleFunc("/ws/simulator/{sid}/notifier", a.validateToken(a.simStatusSocket))

	// websocket used for sending commands to the vehicle
	r.HandleFunc("/ws/vehicle/{sid}/command", a.validateToken(a.ensureControllingVehicle(a.commandVehicle)))

	// websocket used for sending and receiving vehicle viewer messages
	r.HandleFunc("/ws/vehicle/{sid}/view", a.validateToken(a.viewVehicle))

	// websocket used for sending notifications to users
	r.HandleFunc("/ws/operator/notify", a.validateToken(a.notificationWs))

	a.addAuthlessAPIRoutes(r)

	return r
}

func (a *App) addAuthlessAPIRoutes(r *mux.Router) {
	// auth handlers
	r.HandleFunc("/register", a.registerClient).Name("RegisterUser").Methods("POST").GetError()
	r.HandleFunc("/login", a.loginClient).Methods("POST")

	r.HandleFunc("/vehicle/token/{vid}", a.generatePermissionlessToken).Methods("POST")
	// give a vehicle a session cookie for the valid token posted
	r.HandleFunc("/vehicle/auth", a.authVehicleSession).Methods("POST")

	// handler for the robot websocket
	// temp not auth this endpoint until we get the token in the build process
	r.HandleFunc("/ws/vehicle/{vid}/register", a.handleVehicleWS)
}

// CreateRouter create a new mux router and assign all of the routing rules
func (a *App) CreateRouter() *mux.Router {
	mainRouter := mux.NewRouter()

	// Set up index route
	mainRouter.PathPrefix("/app/").HandlerFunc(a.handleTeleopUI)
	mainRouter.HandleFunc("/login", a.loginRoute)
	mainRouter.HandleFunc("/", a.handleTeleopUI)

	// Set up static assets for the react app
	mainRouter.PathPrefix("/static/").Handler(http.FileServer(a.assets.TeleopAssets))

	// Set up route for the old UI
	mainRouter.HandleFunc("/plain", a.handlePlainUI)

	// Set up static assets for the old UI
	mainRouter.PathPrefix("/plain/").Handler(http.StripPrefix("/plain", http.FileServer(a.assets.PlainAssets)))

	// we only want to serve up frames from the file system on a local run
	if a.env == configs.Local {
		mainRouter.PathPrefix("/images/").Handler(http.StripPrefix("/images/", http.FileServer(http.Dir(localImageDir))))
	}

	// handler for the robot websocket
	mainRouter.HandleFunc("/api/teleop/{vid}/vehicle", a.handleVehicleWS) // deprecated

	// Set up the API routes that auth middleware based on env
	a.newAPIRouter(mainRouter)

	// tmp endpoints remove once we're hooked up with the front end
	mainRouter.HandleFunc("/login", serveLogin)
	mainRouter.HandleFunc("/register", serverRegister).Name("serve_register")
	// this route is needed for the ALB to check the health of the server
	mainRouter.HandleFunc("/health", albHealthCheck)

	return mainRouter
}

// temp login page should be deleted when done with testing
func serveLogin(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintln(w, loginPage)
}

// temp register page should be deleted when done with testing
func serverRegister(w http.ResponseWriter, r *http.Request) {
	fmt.Fprintln(w, registerPage)
}

func httpError(w http.ResponseWriter, msg interface{}, parts ...interface{}) {
	s := fmt.Sprintf(fmt.Sprintf("%v", msg), parts...)
	http.Error(w, s, http.StatusInternalServerError)
	log.Println(s)
}

func renderTemplate(w http.ResponseWriter, fs http.FileSystem, path string, data interface{}) {
	f, err := fs.Open(path)
	if err != nil {
		httpError(w, err)
		return
	}
	defer f.Close()

	buf, err := ioutil.ReadAll(f)
	if err != nil {
		httpError(w, err)
		return
	}

	tpl, err := template.New(path).Parse(string(buf))
	if err != nil {
		httpError(w, err)
		return
	}

	err = tpl.Execute(w, data)
	if err != nil {
		httpError(w, err)
		return
	}
}

// frontendParams contains information constructed dynamically on the backend and injected into the frontend.
type frontendParams struct {
	SecureScheme bool
}

func (a *App) handleTeleopUI(w http.ResponseWriter, r *http.Request) {
	// do not cache the asset or else it will not reflect the filesystem state
	// when assets are served from disk

	_, err := services.ValidTokenFromRequest(r, a.secrets.TokenSigningKey)
	if err == nil {
		renderTemplate(w, a.assets.TeleopAssets, "index.html", a.createFrontendParams())
		return
	}

	http.Redirect(w, r, "/login", http.StatusTemporaryRedirect)
}

func (a *App) loginRoute(w http.ResponseWriter, r *http.Request) {
	renderTemplate(w, a.assets.TeleopAssets, "index.html", a.createFrontendParams())
	return
}

func (a *App) createFrontendParams() frontendParams {
	secure := true
	if a.config.Environment == configs.Local {
		secure = false
	}

	return frontendParams{
		SecureScheme: secure,
	}
}

func (a *App) handlePlainUI(w http.ResponseWriter, r *http.Request) {
	// do not cache the asset or else it will not reflect the filesystem state
	// when go-bindata is in debug mode
	_, err := services.ValidTokenFromRequest(r, a.secrets.TokenSigningKey)
	if err == nil {
		renderTemplate(w, a.assets.PlainAssets, "/static/interface.html", frontendParams{})
		return
	}

	http.Redirect(w, r, "/login?next=/plain", http.StatusTemporaryRedirect)
}

func albHealthCheck(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusOK)
}

// NewDebugRouter makes a new router for the heap profiling endpoints
func NewDebugRouter() *mux.Router {
	router := mux.NewRouter()
	router.HandleFunc("/debug/pprof/", pprof.Index)
	router.HandleFunc("/debug/pprof/cmdline", pprof.Cmdline)
	router.HandleFunc("/debug/pprof/profile", pprof.Profile)
	router.HandleFunc("/debug/pprof/symbol", pprof.Symbol)

	// Manually add support for paths linked to by index page at /debug/pprof/
	router.Handle("/debug/pprof/goroutine", pprof.Handler("goroutine"))
	router.Handle("/debug/pprof/heap", pprof.Handler("heap"))
	router.Handle("/debug/pprof/threadcreate", pprof.Handler("threadcreate"))
	router.Handle("/debug/pprof/block", pprof.Handler("block"))

	return router
}
