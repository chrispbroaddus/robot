
#pragma once

#include "packages/hal/include/drivers/gps/gps_device_interface.h"
#include "packages/hal/proto/gps_telemetry.pb.h"
#include "packages/hald/include/device_thread.h"
#include "packages/hald/proto/device_config.pb.h"
#include "packages/net/include/zmq_topic_pub.h"

namespace hald {

class GPSDeviceThread : public DeviceThread {
public:
    GPSDeviceThread(const Device& deviceConfig);
    ~GPSDeviceThread();

    void run() override;

private:
    typedef net::ZMQProtobufPublisher<hal::GPSTelemetry> gps_telemetry_pub_t;

    zmq::context_t m_context;
    std::string m_topic;
    std::shared_ptr<hal::GPSDeviceInterface> m_gps;
    std::unique_ptr<gps_telemetry_pub_t> m_gpsTelemetryPublisher;
};
}
