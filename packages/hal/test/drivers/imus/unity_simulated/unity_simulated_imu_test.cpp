#include "packages/hal/include/drivers/imus/unity_simulated/unity_simulated_imu_device.h"
#include "packages/hal/include/string_utils.h"
#include "packages/net/include/zmq_topic_pub.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

#include <iostream>
#include <thread>

using namespace std;

using namespace hal;

TEST(HalUnitySimulatedImuTest, canCaptureImuSamples) {

    details::property_map_t config;
    config["publisherAddress"] = "tcp://localhost:5701";
    config["cameraId"] = std::to_string(0);
    config["topic"] = "imu";
    config["subscriberHighWaterMark"] = std::to_string(1);
    config["captureTimeoutInMilliseconds"] = std::to_string(1000);

    UnitySimulatedImuDevice imu(4, config);

    IMUSample imuSample;

    EXPECT_NO_THROW(imu.capture(imuSample));
}
