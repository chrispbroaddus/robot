package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"os"

	arg "github.com/alexflint/go-arg"
	"github.com/zippyai/zippy/backend/services"
	"github.com/zippyai/zippy/backend/teleop"
)

func main() {
	var args struct {
		Username    string
		Password    string
		VehicleID   string
		URLBase     string
		Destination string
	}
	args.Username = "admin"
	args.Password = "admin"
	args.URLBase = "http://localhost:8000"
	arg.MustParse(&args)

	url := fmt.Sprintf("%s/api/v1/login", args.URLBase)
	auth, err := getUsersAuthCookie(url, args.Username, args.Password)
	if err != nil {
		log.Fatal("failed to get users auth information: ", err)
	}

	url = fmt.Sprintf("%s/api/v1/vehicle/generate", args.URLBase)
	vehicleAuth, err := requestToken(url, args.VehicleID, auth)
	if err != nil {
		log.Fatal("failed to get the vehicleToken info: ", err)
	}

	if err = writeVehicleTokenFile(args.Destination, vehicleAuth); err != nil {
		log.Fatal("failed to write the token file to disk: ", err)
	}

	log.Printf("created new auth token for vehicle with ID: %s\n", vehicleAuth.VehicleID)
}

func writeVehicleTokenFile(dest string, auth *teleop.VehicleAuthView) error {
	fileName := fmt.Sprintf("%s.jwt", auth.VehicleID)
	if dest != "" {
		fileName = fmt.Sprintf("%s/%s", dest, fileName)
	}

	file, err := os.Create(fileName)
	if err != nil {
		return err
	}
	defer file.Close()

	log.Println("writing token file to: ", fileName)

	_, err = file.WriteString(auth.Token)
	if err != nil {
		return err
	}

	return file.Sync()
}

func requestToken(url, vehicleID string, auth *http.Cookie) (*teleop.VehicleAuthView, error) {
	if vehicleID != "" {
		url = fmt.Sprintf("%s?vehicle_id=%s", url, vehicleID)
	}

	client := http.Client{}
	req, err := http.NewRequest("POST", url, nil)
	if err != nil {
		return nil, err
	}
	req.AddCookie(auth)

	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to get auth token get statusCode: %d", resp.StatusCode)
	}

	tokenData := new(teleop.VehicleAuthView)
	if err = json.NewDecoder(resp.Body).Decode(tokenData); err != nil {
		return nil, err
	}

	return tokenData, nil
}

func getUsersAuthCookie(url, userName, password string) (*http.Cookie, error) {
	auth := &teleop.UserAuth{
		UserName: userName,
		Password: password,
	}

	jBlob, err := json.Marshal(&auth)
	if err != nil {
		return nil, err
	}

	client := http.Client{}
	req, err := http.NewRequest("POST", url, bytes.NewReader(jBlob))
	if err != nil {
		return nil, err
	}

	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("failed to login and get cookie get statusCode: %d", resp.StatusCode)
	}

	cookies := resp.Cookies()
	if len(cookies) < 1 {
		return nil, fmt.Errorf("auth cookie was not present in response")
	}

	for _, cookie := range cookies {
		if cookie.Name == services.AuthKey {
			return cookie, nil
		}
	}

	return nil, fmt.Errorf("no auth cookie with name: %s was found in response", services.AuthKey)
}
