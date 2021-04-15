#!/bin/bash

while getopts n:i:f:u:d: option
do
    case "${option}"
    in
    n) NAME=${OPTARG};;
    i) INTERVAL=${OPTARG};;
    f) SIMULATOR_CONFIG_FILE=${OPTARG};;
    d) DOMAIN=${OPTARG};;
esac
done

# Update the point and go config file to set the name if provided
if [ ! -z ${NAME+x} ]
then
    sed -i '/vehicle_id : / s/"[^"][^"]*"/"'${NAME}'"/' config/global/teleop.pbtxt 
fi

if [ ! -z ${DOMAIN+x} ]
then
    sed -i '/backend_address : / s/"[^"][^"]*"/"ws:\/\/'${DOMAIN}'"/' config/global/teleop.pbtxt 
fi

if [ ! -z ${INTERVAL+x} ]
then
    sed -i '/thumbnail_interval : / s/[^"][^"]*/'${INTERVAL}'/' config/global/point_and_go_options.default.pbtxt 
fi

curl -o /tmp/ZippySimulator.tar.gz https://s3-us-west-1.amazonaws.com/zippysimulator/releases/master/ZippySimulator.tar.gz
tar -zxvf /tmp/ZippySimulator.tar.gz -C /tmp/

# this dep needs to be removed because it breaks the sim run
apt-get remove -y libxrandr2

/tmp/ZippySimulator/sim.x86_64 \
 -settings /zippy/simulator_config/defaults/simulator_settings.json \
 -calibration /zippy/simulator_config/defaults/system_calibration.json &

echo "starting up the simulator this should take a while"
sleep 80s 

# this dep is needed to run point and go
apt-get install -y libgtk2.0-0

# Run hald
bazel-bin/packages/hald/hald -configFile config/11111111-2222-3333-4444-555555555555/hald.conf -logtostderr true &

# Run the point and go application
bazel-bin/applications/point_and_go/point_and_go -camera_calibration /zippy/config/11111111-2222-3333-4444-555555555555/system_calibration.json
