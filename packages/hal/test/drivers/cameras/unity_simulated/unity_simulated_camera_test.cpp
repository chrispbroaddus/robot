#include "packages/hal/include/drivers/cameras/unity_simulated/unity_simulated_camera_device.h"
#include "packages/hal/include/string_utils.h"
#include "packages/net/include/zmq_topic_pub.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

#include <iostream>
#include <thread>

using namespace std;

using namespace hal;

TEST(HalUnitySimulatedCameraTest, canCaptureImages) {

    details::property_map_t config;
    config["publisherAddress"] = "tcp://localhost:5501";
    config["cameraId"] = "FrontLeftStereo";
    config["topic"] = "camera";
    config["subscriberHighWaterMark"] = std::to_string(1);
    config["captureTimeoutInMilliseconds"] = std::to_string(1000);

    UnitySimulatedCameraDevice cam(4, config);

    CameraSample cameraSample;

    EXPECT_NO_THROW(cam.capture(cameraSample));
}
