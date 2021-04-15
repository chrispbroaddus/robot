
#pragma once

#include "gps_device_interface.h"
#include "packages/hal/include/device_factory.h"

namespace hal {

class GPSDevice : public GPSDeviceInterface {
public:
    GPSDevice(const hal::details::property_map_t& config)
        : m_config(config) {
        m_gps = DeviceRegistry<GPSDeviceInterface>::instance().create(config);
    }
    ~GPSDevice() = default;

    bool poll() { return m_gps->poll(); }
    bool capture(hal::GPSTelemetry& telemetry) { return m_gps->capture(telemetry); }

private:
    const hal::details::property_map_t m_config;
    std::shared_ptr<GPSDeviceInterface> m_gps;
};
}
