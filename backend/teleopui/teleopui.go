//go:generate bash -c "cd ../../frontend && yarn build"
//go:generate go-bindata -o bindata.go -pkg teleopui -prefix ../../frontend/build/ ../../frontend/build/...

package teleopui
