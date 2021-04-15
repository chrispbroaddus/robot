package main

import (
	"fmt"
	"log"
	"net/http"

	"github.com/zippyai/zippy/backend/configs"
	"github.com/zippyai/zippy/backend/webhooknode"

	"github.com/alexflint/go-arg"
)

func main() {
	var args struct {
		Config string
	}
	args.Config = "conf/web_hooks/web_hook_server_config.json"
	arg.MustParse(&args)

	config, err := configs.LoadWebHookServerConfig(args.Config)
	if err != nil {
		log.Fatal(err)
	}

	manager := webhooknode.NewWebhookManager(config)

	router := webhooknode.NewWebhookRouter(manager)

	log.Println("starting up the webhook server listening on ", config.Port, "...")
	err = http.ListenAndServe(fmt.Sprintf(":%d", config.Port), router)
	if err != nil {
		log.Fatal(err)
	}
}
