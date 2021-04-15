
#pragma once

#include "packages/hal/include/device_interface.h"
#include "packages/hal/proto/gps_telemetry.pb.h"

namespace hal {

class GPSDeviceInterface : public DeviceInterface {
public:
    GPSDeviceInterface() = default;
    ~GPSDeviceInterface() = default;

    virtual bool poll() = 0;
    virtual bool capture(hal::GPSTelemetry& telemetry) = 0;
};
}
