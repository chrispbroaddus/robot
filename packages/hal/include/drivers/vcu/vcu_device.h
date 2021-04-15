#pragma once

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/vcu/vcu_device_interface.h"

#include <memory>

namespace hal {

namespace vcu {

    /// A mock VCU device
    class VCUDevice : public VCUDeviceInterface {
    public:
        VCUDevice(const hal::details::property_map_t& config)
            : m_config(config) {
            m_vcu = DeviceRegistry<VCUDeviceInterface>::instance().create(config);
        }
        ~VCUDevice() = default;

        bool poll(uint32_t timeoutInMilliseconds) { return m_vcu->poll(timeoutInMilliseconds); }

        bool capture(VCUTelemetryEnvelope& telemetryEnvelope) { return m_vcu->capture(telemetryEnvelope); }

        bool send(const VCUCommandEnvelope& commandEnvelope, VCUCommandResponse& commandResponse) {
            return m_vcu->send(commandEnvelope, commandResponse);
        }

    private:
        const hal::details::property_map_t m_config;
        std::shared_ptr<VCUDeviceInterface> m_vcu;
    };
} // vcu
} // hal
