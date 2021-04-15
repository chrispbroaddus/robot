#pragma once

#include "imu_device_interface.h"
#include "packages/hal/include/device_factory.h"
#include "packages/hal/proto/imu_sample.pb.h"

#include <memory>

namespace hal {

class ImuDevice : public ImuDeviceInterface {
public:
    ImuDevice(const hal::details::property_map_t& config)
        : m_config(config) {
        m_imu = DeviceRegistry<ImuDeviceInterface>::instance().create(config);
    }
    ~ImuDevice() = default;

    bool capture(IMUSample& imuSample) { return m_imu->capture(imuSample); }

private:
    const hal::details::property_map_t m_config;
    std::shared_ptr<ImuDeviceInterface> m_imu;
};
}
