#pragma once

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/network_health/network_health_driver_interface.h"

namespace hal {
class NetworkHealthDriverFactory : public DeviceFactory<NetworkHealthDriverInterface> {
public:
    inline NetworkHealthDriverFactory()
        : DeviceFactory<NetworkHealthDriverInterface>("NetworkHealth") {}

    virtual ~NetworkHealthDriverFactory() = default;

    std::shared_ptr<device_class_t> create(const details::property_map_t& config) final;
};
}