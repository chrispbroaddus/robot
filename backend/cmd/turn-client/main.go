package main

import (
	"io/ioutil"
	"log"
	"net"
	"net/http"

	arg "github.com/alexflint/go-arg"
	"github.com/willscott/goturn/client"
	"github.com/willscott/goturn/common"
)

func main() {
	var args struct {
		Address  string
		Username string
		Password string
	}
	arg.MustParse(&args)

	// Connect to the stun/turn server
	conn, err := net.Dial("tcp", args.Address)
	if err != nil {
		log.Fatal("error dialing TURN server: ", err)
	}
	defer conn.Close()

	credentials := client.LongtermCredentials(args.Username, args.Password)
	client := client.StunClient{Conn: conn}
	_, err = client.Allocate(&credentials)
	if err != nil {
		log.Fatal("failed to obtain dialer: ", err)
	}

	ips, err := net.LookupIP("www.google.com")
	if err != nil {
		log.Fatal("failed to get ip addresses: ", err)
	}

	address := stun.NewAddress("tcp", ips[0], 80)
	c, err := client.Connect(address)
	if err != nil {
		log.Fatal("failed to connect to address: ", err)
	}

	dumbDailer := func(network, address string) (net.Conn, error) {
		return c, nil
	}

	httpClient := &http.Client{Transport: &http.Transport{Dial: dumbDailer}}
	httpResp, err := httpClient.Get("http://www.google.com/")
	if err != nil {
		log.Fatal("error performing http request: ", err)
	}
	defer httpResp.Body.Close()

	httpBody, err := ioutil.ReadAll(httpResp.Body)
	if err != nil {
		log.Fatal("error reading http response: ", err)
	}
	log.Printf("received %d bytes", len(httpBody))
}
