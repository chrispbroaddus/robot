#pragma once

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/imus/imu_device_interface.h"
#include "unity_simulated_imu_device.h"

namespace hal {

class UnitySimulatedImuDeviceFactory : public DeviceFactory<ImuDeviceInterface> {
public:
    UnitySimulatedImuDeviceFactory()
        : DeviceFactory<ImuDeviceInterface>(factoryName()) {}
    ~UnitySimulatedImuDeviceFactory() = default;

    static std::string factoryName() { return "UnitySimulatedImu"; }

    std::shared_ptr<ImuDeviceInterface> create(const hal::details::property_map_t& config) override;
};
}
