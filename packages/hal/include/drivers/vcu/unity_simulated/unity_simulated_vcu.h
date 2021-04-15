#pragma once

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/vcu/vcu_device_interface.h"
#include "packages/net/include/zmq_req_client.h"
#include "packages/net/include/zmq_topic_sub.h"

#include <mutex>
#include <thread>

namespace hal {

namespace vcu {

    /// Driver for the simulated VCU device that communicates with the motherboard through zeromq
    class UnitySimulatedVcu : public VCUDeviceInterface {
    public:
        typedef net::ZMQProtobufSubscriber<VCUTelemetryEnvelope> telemetry_sub_t;
        typedef net::ZMQProtobufReqClient<VCUCommandEnvelope, VCUCommandResponse> command_req_t;

        UnitySimulatedVcu(const hal::details::property_map_t& config);
        ~UnitySimulatedVcu();
        UnitySimulatedVcu(const UnitySimulatedVcu&) = delete;
        UnitySimulatedVcu(const UnitySimulatedVcu&&) = delete;
        UnitySimulatedVcu& operator=(const UnitySimulatedVcu&) = delete;
        UnitySimulatedVcu& operator=(const UnitySimulatedVcu&&) = delete;

        std::string deviceName() const { return "UnitySimulatedVcu"; }
        uint64_t serialNumber() const { return 0; }

        bool poll(uint32_t timeoutInMicroseconds);
        bool capture(VCUTelemetryEnvelope& telemetryEnvelope);
        bool send(const VCUCommandEnvelope& commandEnvelope, VCUCommandResponse& commandResponse);

    private:
        zmq::context_t m_context;
        std::string m_subTopic;
        std::unique_ptr<telemetry_sub_t> m_vehicleTelemetrySubscriber;
        std::unique_ptr<command_req_t> m_CommandReqClient;
    };
} // vcu
} // hal
