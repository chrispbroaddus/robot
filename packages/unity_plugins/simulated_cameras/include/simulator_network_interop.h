#pragma once

#include "packages/hal/proto/camera_id.pb.h"

namespace unity_plugins {

///
/// @brief Interop reset command struct
///
struct SimulatorResetCommand {
    int robot; // reset the robot
    int world; // reset the world
    int streams; // reset the streams
};

///
/// @brief Interop camera configuration command struct
///
struct CameraConfigurationCommand {
    hal::CameraId cameraId; // camera id
    int enable; // enable the camera
    int enableDepth; // enable depth output
    int enablePointcloud; // enable pointcloud output
};

///
/// @brief Interop stats command
///
struct StatsCommand {
    int enable; // enable the stats output
};

///
/// @brief Callbacks to act on the commands
/// These will be functions in C#
///
typedef void (*CameraConfigurationCallback)(CameraConfigurationCommand);
typedef void (*ResetCallback)(SimulatorResetCommand);
typedef void (*StatsCallback)(StatsCommand);

///
/// @brief global network settings
///
struct SimulatorNetworkSettings {
    int zmqSendRecvTimeoutMs; // send/receive timeout in milliseconds
    int zmqLingerTimeMs; // linger time in milliseconds
};

///
/// @brief command server settings
///
struct SimulatorCommandServerSettings {
    char serverAddress[32];
    CameraConfigurationCallback cameraConfigCallback; // camera configuration command callback
    ResetCallback resetCallback; // reset command callback
    StatsCallback statsCallback; // stats command callback
};

///
/// @brief camera publisher settings
///
struct SimulatorCameraPublisherSettings {
    char frontLeftStereoAddress[32];
    char frontLeftStereoDepthAddress[32];
    char frontLeftStereoPointcloudAddress[32];

    char frontRightStereoAddress[32];
    char frontRightStereoDepthAddress[32];
    char frontRightStereoPointcloudAddress[32];

    char rearLeftStereoAddress[32];
    char rearLeftStereoDepthAddress[32];
    char rearLeftStereoPointcloudAddress[32];

    char rearRightStereoAddress[32];
    char rearRightStereoDepthAddress[32];
    char rearRightStereoPointcloudAddress[32];

    char frontFisheyeAddress[32];
    char frontFisheyeDepthAddress[32];
    char frontFisheyePointcloudAddress[32];

    char rearFisheyeAddress[32];
    char rearFisheyeDepthAddress[32];
    char rearFisheyePointcloudAddress[32];

    char leftFisheyeAddress[32];
    char leftFisheyeDepthAddress[32];
    char leftFisheyePointcloudAddress[32];

    char rightFisheyeAddress[32];
    char rightFisheyeDepthAddress[32];
    char rightFisheyePointcloudAddress[32];

    int zmqHighWaterMarkValue; // high water mark value
};

///
/// @brief stats publisher settings
///
struct SimulatorStatsPublisherSettings {
    char statsAddress[32];
    int zmqHighWaterMarkValue; // high water mark value
};

///
/// @brief settings used for the networking
///
struct SimulatorNetworkManagerSettings {
    SimulatorNetworkSettings network; // network settings
    SimulatorCommandServerSettings commandServer; // command server settings
    SimulatorCameraPublisherSettings cameraPublisher; // camera publisher settings
    SimulatorStatsPublisherSettings statsPublisher; // stats publisher settings
};
} // namespace unity_plugins
