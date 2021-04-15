#pragma once

#include "packages/hal/include/device_interface.h"
#include "packages/hal/proto/imu_sample.pb.h"

namespace hal {

class ImuDeviceInterface : public DeviceInterface {
public:
    ImuDeviceInterface() = default;
    ~ImuDeviceInterface() = default;

    /// Capture an imu sample from an imu device
    virtual bool capture(IMUSample& imuSample) = 0;
};
}
