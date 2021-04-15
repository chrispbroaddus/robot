#pragma once

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/vcu/vcu_device_interface.h"

namespace hal {

namespace vcu {

    /// A factory that creates UDP socket based VCU drivers
    class UdpSocketVcuDriverFactory : public DeviceFactory<VCUDeviceInterface> {
    public:
        UdpSocketVcuDriverFactory()
            : DeviceFactory<VCUDeviceInterface>(factoryName()) {}
        ~UdpSocketVcuDriverFactory() = default;

        static std::string factoryName() { return "UdpSocketVcuDriver"; }

        std::shared_ptr<VCUDeviceInterface> create(const hal::details::property_map_t& config) override;
    };
} // vcu
} // hal
