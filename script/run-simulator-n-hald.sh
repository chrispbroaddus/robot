#!/bin/bash

if [[ -z "${CIRCLE_BRANCH}" ]] ; then
  CIRCLE_BRANCH=master
fi

# Download & extract Simulator
SIMULATOR_TAR_FILE="ZippySimulator.tar.gz"
SIMULATOR_URL="https://s3-us-west-1.amazonaws.com/zippysimulator/releases/$CIRCLE_BRANCH/$SIMULATOR_TAR_FILE"
wget -N -q --show-progress -P /tmp/ $SIMULATOR_URL
tar -zxvf /tmp/$SIMULATOR_TAR_FILE -C /tmp/

# Run simulator
IP_ADDRESS="localhost"
CAMERA_CONFIG_PORT="5401"
TRAJECTORY_PORT="5803"
PUBLISH_CAM0_PORT="5501"
PUBLISH_DEPTH0_PORT="5601"
SKIT_STEERING="0"
GPS_PORT="5700"
VCU_TELEMETRY_PORT="7001"
UNITY_TELEMETRY_PORT="7501"
APRILTAG_POSES_PORT="7601"

v1="-publishIpAddress $IP_ADDRESS"
v2="-cameraConfigPort $CAMERA_CONFIG_PORT"
v3="-trajectoryPort $TRAJECTORY_PORT"
v4="-publishCam0Port $PUBLISH_CAM0_PORT"
v5="-publishDepth0Port $PUBLISH_DEPTH0_PORT"
v6="-skitSteering $SKIT_STEERING"
v7="-gpsPort $GPS_PORT"
v8="-vcuTelemetryPort $VCU_TELEMETRY_PORT"
v9="-unityTelemetryPort $UNITY_TELEMETRY_PORT"
v10="-apriltagPosesPort $APRILTAG_POSES_PORT"

/tmp/ZippySimulator/sim.x86_64 \
 -publishIpAddress $IP_ADDRESS \
 -cameraConfigPort $CAMERA_CONFIG_PORT \
 -trajectoryPort $TRAJECTORY_PORT \
 -publishCam0Port $PUBLISH_CAM0_PORT \
 -publishDepth0Port $PUBLISH_DEPTH0_PORT \
 -skitSteering $SKIT_STEERING \
 -gpsPort $GPS_PORT \
 -vcuTelemetryPort $VCU_TELEMETRY_PORT \
 -unityTelemetryPort $UNITY_TELEMETRY_PORT \
 -apriltagPosesPort $APRILTAG_POSES_PORT &
 
sleep 2s

printf "\n\n\n"
printf "============================================================================\n"
printf "Once the simulator is running, press enter to continue.\n"
printf "(WARNING: Please do not attempt to continue before the simulator is active.)\n"
printf "============================================================================\n"

read

# Build hald
bazel build //packages/hald

# Run hald
bazel-bin/packages/hald/hald -configFile config/11111111-2222-3333-4444-555555555555/hald.conf -logtostderr true

