
#pragma once

#include "packages/hald/include/device_thread.h"
#include "packages/hald/proto/device_config.pb.h"

#include "packages/hal/include/drivers/joysticks/joystick_device_interface.h"
#include "packages/hal/proto/joystick_sample.pb.h"

#include "packages/net/include/zmq_topic_pub.h"

namespace hald {

class JoystickDeviceThread : public DeviceThread {
public:
    JoystickDeviceThread(const Device& deviceConfig);
    ~JoystickDeviceThread();

    void run() override;

private:
    typedef net::ZMQProtobufPublisher<hal::JoystickSample> joystick_pub_t;

    zmq::context_t m_context;
    std::string m_topic;
    std::shared_ptr<hal::JoystickDeviceInterface> m_joystick;
    std::unique_ptr<joystick_pub_t> m_joystickSamplePublisher;
};
}