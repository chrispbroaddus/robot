
#pragma once

#include "packages/hald/include/device_thread.h"
#include "packages/hald/proto/device_config.pb.h"

#include "packages/hal/include/drivers/network_health/network_health_driver_interface.h"
#include "packages/hal/proto/network_health_telemetry.pb.h"

#include "packages/net/include/zmq_topic_pub.h"

namespace hald {

class NetworkHealthDeviceThread : public DeviceThread {
public:
    NetworkHealthDeviceThread(const Device& deviceConfig);
    ~NetworkHealthDeviceThread();

    void run() override;

private:
    using publisher_type = net::ZMQProtobufPublisher<hal::NetworkHealthTelemetry>;

    zmq::context_t m_context;
    std::string m_topic;
    std::shared_ptr<hal::NetworkHealthDriverInterface> m_driver;
    std::unique_ptr<publisher_type> m_publisher;
};
}