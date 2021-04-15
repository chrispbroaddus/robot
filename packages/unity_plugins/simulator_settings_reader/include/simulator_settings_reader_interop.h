#pragma once

#include "packages/hal/proto/camera_id.pb.h"
#include "packages/unity_plugins/utils/include/vector2.h"
#include "packages/unity_simulator/proto/simulator_settings.pb.h"

namespace unity_plugins {

struct VehicleCalibration {
    Vector2 center;
    Vector2 frontRight;
    Vector2 frontLeft;
    Vector2 middleRight;
    Vector2 middleLeft;
    Vector2 rearRight;
    Vector2 rearLeft;
};

struct VehicleSettings {
    simulator_settings::SimulatedVehicleModelId modelId;
    VehicleCalibration vehicleCalibration;
};

struct ApplicationSettings {
    int targetFrameRate;
    float targetFixedDeltaTime;
};

struct CameraSettings {
    hal::CameraId cameraId;
    int greyscale;
    float nearClippingPlaneDistanceMeters;
    float farClippingPlaneDistanceMeters;
    float distortedMeshRowFineness;
    float distortedMeshColFineness;
    float calibratedPlaneScaleFactor;
    float maxDepthMapDistanceMeters;
    float imageScale;
    float imageZoom;
    int enabled;
    int depthEnabled;
    int pointcloudEnabled;
};

struct CamerasSettings {
    int frameDownloadBufferSize;
    float cameraStartDepth;
    int maxTextureSize;
    CameraSettings camera[hal::CameraId_ARRAYSIZE];
};

struct NetworkingSettings {
    int zmqSendReceiveTimeoutMs;
    int zmqLingerTimeMs;
    int zmqHighWaterMark;
};

struct GroundTruthSettings {
    int publishVehiclePosition;
    int publishFiducialPoses;
};

struct SceneSettings {
    char sceneUrl[2048];
};

struct SimulatorSettings {
    VehicleSettings vehicle;
    ApplicationSettings application;
    CamerasSettings cameras;
    NetworkingSettings networking;
    GroundTruthSettings groundTruth;
    SceneSettings scene;
};
}