#pragma once

#include "packages/hal/include/drivers/vcu/unity_simulated/unity_simulated_vcu.h"
#include "packages/hald/include/device_thread.h"
#include "packages/hald/proto/device_config.pb.h"
#include "packages/net/include/zmq_rep_server.h"
#include "packages/net/include/zmq_topic_pub.h"

namespace hald {

class VCUDeviceThread : public DeviceThread {
public:
    VCUDeviceThread(const Device& deviceConfig);
    ~VCUDeviceThread();

    void run() override;

private:
    typedef net::ZMQProtobufPublisher<hal::VCUTelemetryEnvelope> telemetry_pub_t;
    typedef net::ZMQProtobufRepServer<hal::VCUCommandEnvelope, hal::VCUCommandResponse> trajectory_rep_t;

    zmq::context_t m_context;
    std::string m_pubTopic;
    std::shared_ptr<hal::vcu::VCUDeviceInterface> m_vcu;
    std::unique_ptr<telemetry_pub_t> m_vehicleTelemetrySamplePublisher;
    std::unique_ptr<trajectory_rep_t> m_trajectorySampleRepServer;
    std::uint32_t m_telemetryTimeoutInMilliseconds;
};
}
