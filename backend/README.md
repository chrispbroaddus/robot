## Install go 1.8

```shell
mkdir $HOME/go # set this to $GOPATH in your .bashrc
curl -O https://storage.googleapis.com/golang/go1.8.1.darwin-amd64.tar.gz
sudo tar -C /usr/local/ -xzf go1.8.1.darwin-amd64.tar.gz
```

Then add to your `.bashrc`:

```shell
export GOPATH=$HOME/go
export PATH=$PATH:$GOROOT/bin:$GOPATH/bin
```

## Put zippy into GOPATH:

```
mkdir -p $GOPATH/src/github.com/zippyai
mv zippy $GOPATH/src/github.com/zippyai/zippy
```

You may then wish to symlink zippy into its original location:

```
ln -s $GOPATH/src/github.com/zippyai/zippy
```

But note that you cannot do the symlink the other way around (i.e. link from GOPATH to original location) since `go build` does not respect symlinks.

## Run the backend

```
cd backend
go generate
go build ./cmd/teleop-server
./teleop-server -port 19870 -env 1
```
defaults for port is 8000
deafult for env is 1 which stands for localhost
env values are as follows
0: testing environment no auth and local storage
1: local environment auth but local storage
2: staging environment auth with permanent storage not implemented
3: production environment auth with permanent storage not implemented

To reconfigure the server:
```
cp env/prod env/local
# edit env/local
env/local ./teleop-server
```

## Run the mock vehicle

This is a mock of the vehicle. It connects to the backend and sends images to
it as if those images were coming from the camera on the vehicle.

First create a directory with two or more jpeg images in it.

Next check that the backend is running.

Now run the mock vehicle:
```
cd backend
go build ./cmd/mock-vehicle
./mock-vehicle --frames <DIR>   # dir containing jpegs
```

## Optional: generate vehicle auth file

This is a tool that will hit the teleop-server to sign a jwt key file and write it locally.
This will be used for authenticating requests made from the vehicle to the teleop-server.

1. Make sure that the backend is running.
2. Run the generator tool
	```
	go build ./cmd/vehicle-auth
	./vehicle-auth
	```

	This will hit server at localhost:8000 and generate a new file for a new vehicle in the current dir.
	There are more options available to you use -h to see the help menu.

## Optional: regenerate protobufs

1. Install protobuf compiler: 
2. Install the golang plugin for the protobuf compiler:
	```
	go get -u github.com/golang/protobuf/protoc-gen-go
	```
3. Generate protobufs:
	```
	script/generate-protobufs
	```
