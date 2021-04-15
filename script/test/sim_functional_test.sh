#!/bin/bash
# Simulator functional tests, Version : 0.0.4
#
# It validates the expected high-level behavior of the simulator,
# such as interaction between commands and its reaction from the simulator.
#
# Details : https://zippy.quip.com/N9RJAg4TVdZw/Simulator-Functional-Test
#
# For usage, please run from root folder of /zippy repo.

UNITY_COMPILER="/opt/Unity/Editor/Unity"

if [ "$#" -ge 2 ]; then
    echo " ## Usage :"
    echo " ## ./script/test/sim_functional_test.sh [UNITY_COMPILER]"
    exit 1
fi

if [ "$#" = 1 ]; then
    UNITY_COMPILER=$1
fi

if ! [ -f $UNITY_COMPILER ]; then
    echo "Unity is not installed, or cannot find the Unity compiler from $UNITY_COMPILER."
    exit 1
fi

TEST_SIM_PATH="/tmp/ZippySimulatorFunctionalTest"

if [ -d "$DIRECTORY" ]; then
  rm -rf $TEST_SIM_PATH
fi

mkdir -p $TEST_SIM_PATH

bazel build //packages/unity_simulator
/opt/Unity/Editor/Unity -batchmode -nographics -buildLinux64Player $TEST_SIM_PATH/sim \
-projectPath $PWD/packages/unity_simulator/ZippySimUnity/ -quit



#######################################################################################
# Each block runs a new sim instance to avoid side-effect coming from the other tests
#######################################################################################

###
timeout 60 $TEST_SIM_PATH/sim &
SIM_PID=$!
bazel test //packages/unity_simulator/test:vcu_telemetry_receive_test
kill $SIM_PID
sleep 3
###

###
timeout 90 $TEST_SIM_PATH/sim &
SIM_PID=$!
sleep 15 # sleep for 15 seconds to make sure the simulator is fully activated.
bazel test //packages/unity_simulator/test:wheel_odometry_test
kill $SIM_PID
sleep 3
###

###
timeout 90 $TEST_SIM_PATH/sim &
SIM_PID=$!
sleep 15 # sleep for 15 seconds to make sure the simulator is fully activated.
bazel test //packages/unity_simulator/test:servo_telemetry_test
kill $SIM_PID
sleep 3
###

###
timeout 90 $TEST_SIM_PATH/sim &
SIM_PID=$!
sleep 15 # sleep for 15 seconds to make sure the simulator is fully activated.
bazel test //packages/unity_simulator/test:hald_compatability_test
kill $SIM_PID
sleep 3
###

###
timeout 60 $TEST_SIM_PATH/sim &
SIM_PID=$!
bazel test //packages/unity_simulator/test:imu_test
kill $SIM_PID
sleep 3
###

###
timeout 60 $TEST_SIM_PATH/sim &
SIM_PID=$!
bazel test //packages/unity_simulator/test:gps_test
kill $SIM_PID
sleep 3
###


#######################################
# Delete the temporary folders
#######################################
rm -rf $TEST_SIM_PATH
