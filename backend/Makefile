PKGS := $(shell go list ./... | grep -v '/vendor/')

ci: fmt vet lint generate test

deps:
	go get github.com/golang/protobuf/proto
	go get github.com/golang/protobuf/protoc-gen-go
	go get github.com/golang/lint/golint
	go get golang.org/x/tools/cmd/goimports
	go get github.com/jteeuwen/go-bindata/...

fmt:
	find . -name '*.go' | grep -v '/vendor/' | grep -v '.pb.go' | grep -v 'bindata.go' | xargs -I path goimports -l=true path > /tmp/fmt.out
	cat /tmp/fmt.out
	! test -s /tmp/fmt.out

vet:
	find * -maxdepth 0 -type d | grep -v vendor | xargs go tool vet

lint:
	find . -name '*.go' | grep -v '/vendor/' | grep -v '.pb.go' | grep -v 'bindata.go' | xargs -I path golint path > /tmp/lint.out
	cat /tmp/lint.out
	! test -s /tmp/lint.out

build: generate
	go build -v $(PKGS)

generate:
	go generate ./teleopui/teleopui.go

test:
	go test -v -cover -timeout 2m $(PKGS)
	# race detector is slow so only run it on goroutine-heavy packages
	go test -v -race \
		./teleop \
		./teleopnode \
		./simulatornode
