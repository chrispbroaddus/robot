#pragma once

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/gps/gps_device_interface.h"

namespace hal {

class GPSDDeviceFactory : public DeviceFactory<GPSDeviceInterface> {
public:
    GPSDDeviceFactory()
        : DeviceFactory<GPSDeviceInterface>(factoryName()) {}
    ~GPSDDeviceFactory() = default;

    static std::string factoryName() { return "GPSDDeviceFactory"; }

    std::shared_ptr<GPSDeviceInterface> create(const hal::details::property_map_t& config) override;
};
}
