#pragma once

#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"

namespace perception {

class FiducialDetectorInterface {
public:
    FiducialDetectorInterface() = default;
    ~FiducialDetectorInterface() = default;
    FiducialDetectorInterface(const FiducialDetectorInterface&) = delete;
    FiducialDetectorInterface(const FiducialDetectorInterface&&) = delete;
    FiducialDetectorInterface& operator=(const FiducialDetectorInterface&) = delete;
    FiducialDetectorInterface& operator=(const FiducialDetectorInterface&&) = delete;

    virtual bool estimatePose(const hal::CameraSample& cameraSample, std::vector<calibration::CoordinateTransformation>& detectedPoses) = 0;
};
} // perception
