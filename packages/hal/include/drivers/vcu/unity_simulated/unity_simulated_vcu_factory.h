#pragma once

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/vcu/unity_simulated/unity_simulated_vcu.h"
#include "packages/hal/include/drivers/vcu/vcu_device_interface.h"

namespace hal {

namespace vcu {

    /// A factory that creates a Unity simulated VCU device
    class UnitySimulatedVcuFactory : public DeviceFactory<VCUDeviceInterface> {
    public:
        UnitySimulatedVcuFactory()
            : DeviceFactory<VCUDeviceInterface>(factoryName()) {}

        ~UnitySimulatedVcuFactory() = default;

        static std::string factoryName() { return "UnitySimulatedVcu"; }

        std::shared_ptr<VCUDeviceInterface> create(const hal::details::property_map_t& config) override;
    };
} // vcu
} // hal
