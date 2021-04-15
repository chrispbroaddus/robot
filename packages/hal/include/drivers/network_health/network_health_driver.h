#pragma once

#include "packages/hal/include/drivers/network_health/network_health_driver_interface.h"

#include <string>
#include <vector>

namespace hal {
class NetworkHealthDriver : public NetworkHealthDriverInterface {
public:
    inline explicit NetworkHealthDriver(const std::vector<std::string>& tgts)
        : targets(tgts) {}

    inline explicit NetworkHealthDriver(std::vector<std::string>&& tgts)
        : targets(std::move(tgts)) {}

    NetworkHealthTelemetry sample();

    // There will only ever be one instance of network health driver
    inline std::string deviceName() const final { return "NetworkHealth"; }

    // There will only ever be one instance of network health driver
    inline uint64_t serialNumber() const final { return 0; }

private:
    std::vector<std::string> targets;
};
}