package main

import (
	"fmt"
	"log"
	"log/syslog"
	"net/http"
	"os"

	arg "github.com/alexflint/go-arg"
	"github.com/prometheus/client_golang/prometheus/promhttp"
	"github.com/rs/cors"
	"github.com/zippyai/zippy/backend/configs"
	"github.com/zippyai/zippy/backend/simulatornode"
)

const (
	syslogAddrKey = "LOGGLY_PORT_514_UDP_ADDR"
	syslogPortKey = "LOGGLY_PORT_514_UDP_PORT"
)

func main() {
	var args struct {
		Config  string
		Secrets string
	}
	args.Config = "conf/sim_host/dev_config.json"
	args.Secrets = "conf/sim_host/dev_secrets.json"
	arg.MustParse(&args)

	// generate the config values from the file passed in
	config, err := configs.LoadConfig(args.Config)
	if err != nil {
		log.Fatal("fatal error trying to load config file: ", err)
	}

	if config.Environment != configs.Local {
		// enable syslogging for all application logging if we're in a prod env
		syslogAddress := os.Getenv(syslogAddrKey)
		syslogPort := os.Getenv(syslogPortKey)
		logwriter, err := syslog.Dial("udp", fmt.Sprintf("%s:%s", syslogAddress, syslogPort), syslog.LOG_NOTICE, "teleop-server")
		if err != nil {
			log.Fatal("fatal error trying to set up syslogging, ", err)
		}
		log.SetOutput(logwriter)
	}

	secrets, err := configs.LoadSecrets(args.Secrets)
	if err != nil {
		log.Fatal("fatal error trying to load secrets file: ", err)
	}

	opts, err := simulatornode.NewSimHostOptions(config, secrets)
	if err != nil {
		log.Fatal(err)
	}

	sessionManager := simulatornode.NewSessionManager(opts)

	router := sessionManager.NewSimulatorHostRouter()

	// Enable CORS in our responses
	handler := cors.New(cors.Options{
		AllowedOrigins:   []string{"*"},
		AllowedMethods:   []string{"GET", "POST", "DELETE", "OPTIONS", "PUT"},
		AllowCredentials: true,
	}).Handler(router)

	go func() {
		log.Println("starting up the metrics endpoint listening on ", config.MetricsPort, "...")
		mux := http.NewServeMux()
		mux.Handle("/metrics", promhttp.Handler())
		err = http.ListenAndServe(fmt.Sprintf(":%d", config.MetricsPort), mux)
	}()

	log.Println("listening on", config.Port, "...")
	err = http.ListenAndServe(fmt.Sprintf(":%d", config.Port), handler)
	if err != nil {
		log.Fatal(err)
	}
}
