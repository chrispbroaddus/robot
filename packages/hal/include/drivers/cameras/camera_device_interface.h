
#pragma once

#include "packages/hal/include/device_interface.h"
#include "packages/hal/proto/camera_sample.pb.h"

namespace hal {

class CameraDeviceInterface : public DeviceInterface {
public:
    CameraDeviceInterface() = default;
    ~CameraDeviceInterface() = default;

    /// Capture an image from the camera
    virtual bool capture(CameraSample& cameraSample) = 0;

    /// Set Auto-Exposure RoI based on User Input
    virtual bool setAutoExposureRoi(float xFraction, float yFraction, float radiusFraction) = 0;
};
}
