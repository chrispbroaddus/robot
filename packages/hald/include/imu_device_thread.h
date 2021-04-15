#pragma once

#include "packages/hal/include/drivers/imus/imu_device_interface.h"
#include "packages/hal/proto/imu_sample.pb.h"
#include "packages/hald/include/device_thread.h"
#include "packages/hald/proto/device_config.pb.h"
#include "packages/net/include/zmq_topic_pub.h"

namespace hald {

class ImuDeviceThread : public DeviceThread {
public:
    ImuDeviceThread(const Device& deviceConfig);
    ~ImuDeviceThread();

    void run() override;

private:
    typedef net::ZMQProtobufPublisher<hal::IMUSample> imu_sample_pub_t;

    zmq::context_t m_context;
    std::string m_topic;
    std::shared_ptr<hal::ImuDeviceInterface> m_imu;
    std::unique_ptr<imu_sample_pub_t> m_imuSamplePublisher;
};
}
