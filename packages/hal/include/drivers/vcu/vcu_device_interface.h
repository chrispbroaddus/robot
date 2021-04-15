#pragma once

#include "packages/hal/include/device_interface.h"
#include "packages/hal/proto/vcu_command_envelope.pb.h"
#include "packages/hal/proto/vcu_command_response.pb.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"

#include <thread>

namespace hal {

namespace vcu {

    /// VCUDeviceInterface defines the interface for all VCU devices
    class VCUDeviceInterface : public DeviceInterface {
    public:
        VCUDeviceInterface() = default;
        ~VCUDeviceInterface() = default;

        /// Poll function to check whether telemetry data is  available
        virtual bool poll(uint32_t timeoutInMicroseconds) = 0;

        /// Receives telemetry data from the VCU and passes it to the calling function
        virtual bool capture(VCUTelemetryEnvelope& telemetryEnvelope) = 0;

        /// Sends the VCU a command and returns the response generated by the VCU
        virtual bool send(const VCUCommandEnvelope& commandEnvelope, VCUCommandResponse& commandResponse) = 0;
    };
} // vcu
} // hal
