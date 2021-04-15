#include "packages/hal/include/drivers/cameras/unity_simulated/unity_simulated_camera_device_factory.h"
#include "packages/net/include/zmq_rep_server.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "gtest/gtest.h"

#include <thread>

using namespace hal;

TEST(HalUnitySimulatedCameraFactoryTest, canInstantiate) {
    EXPECT_NO_THROW(UnitySimulatedCameraDeviceFactory s_UnitySimultedCameraFactory);
}

TEST(HalUnitySimulatedCameraFactoryTest, noCameraId) {

    UnitySimulatedCameraDeviceFactory unitySimulatedCameraDeviceFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["publisherAddress"] = "tcp://localhost:5501";
    // missing : deviceConfig["cameraId"] = "FrontLeftStereo";
    deviceConfig["topic"] = "camera";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_THROW(unitySimulatedCameraDeviceFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUnitySimulatedCameraFactoryTest, noTopic) {

    UnitySimulatedCameraDeviceFactory unitySimulatedCameraDeviceFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["publisherAddress"] = "tcp://localhost:5501";
    deviceConfig["cameraId"] = "FrontLeftStereo";
    // missing : deviceConfig["topic"] = "camera";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_THROW(unitySimulatedCameraDeviceFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUnitySimulatedCameraFactoryTest, noServerAddress) {

    UnitySimulatedCameraDeviceFactory unitySimulatedCameraDeviceFactory;

    hal::details::property_map_t deviceConfig;
    // missing : deviceConfig["publisherAddress"] = "tcp://localhost:5501";
    deviceConfig["cameraId"] = "FrontLeftStereo";
    deviceConfig["topic"] = "camera";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_THROW(unitySimulatedCameraDeviceFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUnitySimulatedCameraFactoryTest, noSubscriberHighWaterMark) {

    UnitySimulatedCameraDeviceFactory unitySimulatedCameraDeviceFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["publisherAddress"] = "tcp://localhost:5501";
    deviceConfig["cameraId"] = "FrontLeftStereo";
    deviceConfig["topic"] = "camera";
    // missing : deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_THROW(unitySimulatedCameraDeviceFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUnitySimulatedCameraFactoryTest, noCaptureTimeout) {

    UnitySimulatedCameraDeviceFactory unitySimulatedCameraDeviceFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["publisherAddress"] = "tcp://localhost:5501";
    deviceConfig["cameraId"] = "FrontLeftStereo";
    deviceConfig["topic"] = "camera";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    // missing : deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_THROW(unitySimulatedCameraDeviceFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUnitySimulatedCameraFactoryTest, canCreateCamera) {

    UnitySimulatedCameraDeviceFactory unitySimulatedCameraDeviceFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["publisherAddress"] = "tcp://localhost:5501";
    deviceConfig["cameraId"] = "FrontLeftStereo";
    deviceConfig["topic"] = "camera";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_NO_THROW(unitySimulatedCameraDeviceFactory.create(deviceConfig));
}
