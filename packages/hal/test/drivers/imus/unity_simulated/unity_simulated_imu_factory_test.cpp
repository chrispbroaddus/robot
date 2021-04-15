#include "packages/hal/include/drivers/imus/unity_simulated/unity_simulated_imu_device_factory.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "gtest/gtest.h"

#include <thread>

using namespace hal;

TEST(HalUnitySimulatedImuFactoryTest, canInstantiate) { EXPECT_NO_THROW(UnitySimulatedImuDeviceFactory unitySimulatedImuDeviceFactory); }

TEST(HalUnitySimulatedImuFactoryTest, noImuId) {

    UnitySimulatedImuDeviceFactory unitySimulatedImuDeviceFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["publisherAddress"] = "tcp://localhost:5701";
    // missing : deviceConfig["imuId"] = std::to_string(0);
    deviceConfig["topic"] = "imu";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_THROW(unitySimulatedImuDeviceFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUnitySimulatedImuFactoryTest, noTopic) {

    UnitySimulatedImuDeviceFactory unitySimulatedImuDeviceFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["publisherAddress"] = "tcp://localhost:5701";
    deviceConfig["imuId"] = std::to_string(0);
    // missing : deviceConfig["topic"] = "imu";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_THROW(unitySimulatedImuDeviceFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUnitySimulatedImuFactoryTest, noServerAddress) {

    UnitySimulatedImuDeviceFactory unitySimulatedImuDeviceFactory;

    hal::details::property_map_t deviceConfig;
    // missing : deviceConfig["publisherAddress"] = "tcp://localhost:5701";
    deviceConfig["imuId"] = std::to_string(0);
    deviceConfig["topic"] = "imu";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_THROW(unitySimulatedImuDeviceFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUnitySimulatedImuFactoryTest, noSubscriberHighWaterMark) {

    UnitySimulatedImuDeviceFactory unitySimulatedImuDeviceFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["publisherAddress"] = "tcp://localhost:5701";
    deviceConfig["imuId"] = std::to_string(0);
    deviceConfig["topic"] = "imu";
    // missing : deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_THROW(unitySimulatedImuDeviceFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUnitySimulatedImuFactoryTest, noCaptureTimeout) {

    UnitySimulatedImuDeviceFactory unitySimulatedCameraDeviceFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["publisherAddress"] = "tcp://localhost:5701";
    deviceConfig["imuId"] = std::to_string(0);
    deviceConfig["topic"] = "imu";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    // missing : deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_THROW(unitySimulatedCameraDeviceFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUnitySimulatedImuFactoryTest, canCreateImu) {

    UnitySimulatedImuDeviceFactory unitySimulatedImuDeviceFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["publisherAddress"] = "tcp://localhost:5701";
    deviceConfig["imuId"] = std::to_string(0);
    deviceConfig["topic"] = "imu";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_NO_THROW(unitySimulatedImuDeviceFactory.create(deviceConfig));
}
