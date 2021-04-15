package data

import (
	metrics "github.com/go-kit/kit/metrics/prometheus"
	"github.com/prometheus/client_golang/prometheus"
)

const (
	nameSpace = "mission_control"
	subSystem = "data"

	// data stores
	redisStore = "redis"
	postgres   = "postgres"

	// vehicle methods
	authVehicle            = "auth_vehicle"
	createVehicle          = "create_vehicle"
	createVehicleAuth      = "create_vehicle_auth"
	updateVehicleAuth      = "update_vehicle_auth"
	setVehicleLocation     = "set_vehicle_location"
	saveCameraFrame        = "save_camera_frame"
	setVehicleCameras      = "save_vehicle_cameras"
	findVehicle            = "find_vehicle"
	getVehicleCache        = "get_vehicle_cache"
	setVehicleCache        = "set_vehicle_cache"
	getVehicleCameraCache  = "get_vehicle_camera_cache"
	getVehicleCamerasCache = "get_vehicle_cameras_cache"
	setVehicleCameraCache  = "set_vehicle_camera_cache"
	// user methods
	createUserAuth = "create_user_auth"
	updateUserAuth = "update_user_auth"
	setAdmin       = "set_admin"
	findUser       = "find_user"
	findUserByName = "find_user_by_name"
	// simulator methods
	simulatorNames      = "simulator_names"
	simulatorInfo       = "simulator_info"
	addSimulatorType    = "add_simulator_type"
	removeSimulatorType = "remove_simulator_type"
	// sim host methods
	setSimSession         = "set_sim_session"
	updateSimSessionState = "update_sim_session_state"
	findSimSession        = "find_sim_session"
	removeSimSession      = "remove_sim_session"
	createRecipeBook      = "create_recipe_book"
	removeRecipeBook      = "remove_recipe_book"
	removeRecipe          = "remove_recipe"
	addRecipe             = "add_recipe"
	findRecipeBook        = "find_recipe_book"
)

var (
	// counters
	operationsErrorTotal *metrics.Counter
	// histograms
	requestTime *metrics.Histogram
)

func init() {
	operationsErrorTotal = metrics.NewCounterFrom(prometheus.CounterOpts{
		Namespace: nameSpace,
		Subsystem: subSystem,
		Name:      "data_operations_error_total",
		Help:      "The number of errors we got on operations with the data services",
	}, []string{"error", "method", "data_store"})

	requestTime = metrics.NewHistogramFrom(prometheus.HistogramOpts{
		Namespace: nameSpace,
		Subsystem: subSystem,
		Name:      "request_times",
		Help:      "The amount of time it takes to complete a request with the data  services",
		Buckets:   prometheus.LinearBuckets(10, 15, 10), // start at 10 ms with 10 buckets 15 ms apart
	}, []string{"method", "data_store"})
}
