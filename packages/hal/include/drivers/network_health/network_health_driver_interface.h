#pragma once

#include "packages/hal/include/device_interface.h"
#include "packages/hal/proto/network_health_telemetry.pb.h"

namespace hal {
class NetworkHealthDriverInterface : public DeviceInterface {
public:
    virtual ~NetworkHealthDriverInterface() = default;

    virtual NetworkHealthTelemetry sample() = 0;
};
}