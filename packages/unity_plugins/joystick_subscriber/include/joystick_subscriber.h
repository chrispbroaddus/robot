#pragma once

#include "packages/hal/proto/joystick_sample.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace unity_plugins {

class JoystickSubscriber {
public:
    struct JoystickAxisResponse {
        int success = 0;
        float axis0;
        float axis1;
    };

    ///
    /// @brief ZMQ subscriber listening to joystick input
    /// \param addr
    /// \param topic
    /// \param port
    /// \param timeoutInMilliseconds
    ///
    JoystickSubscriber(const std::string& addr, const int highWaterMark, const int axisSize, const double maxLatencyInSec);

    JoystickSubscriber(const JoystickSubscriber& obj) = delete;
    JoystickSubscriber& operator=(JoystickSubscriber other) = delete;

    ~JoystickSubscriber() = default;

    bool receive(hal::JoystickSample& sample);

private:
    zmq::context_t m_context;
    double m_maxLatencyInSec;
    net::ZMQProtobufSubscriber<hal::JoystickSample> m_subscriber;
};
} // namespace unity_plugins