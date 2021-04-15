#pragma once

#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/net/include/zmq_topic_pub.h"

#include <unordered_map>

namespace unity_plugins {

class VcuTelemetryPublisher {

public:
    struct WheelsRadius {
        float leftFront;
        float leftMiddle;
        float leftRear;
        float rightFront;
        float rightMiddle;
        float rightRear;

        WheelsRadius(const float newLeftFront, const float newLeftMiddle, const float newLeftRear, const float newRightFront,
            const float newRightMiddle, const float newRightRear) {
            leftFront = newLeftFront;
            leftMiddle = newLeftMiddle;
            leftRear = newLeftRear;
            rightFront = newRightFront;
            rightMiddle = newRightMiddle;
            rightRear = newRightRear;
        };
    };

    ///
    /// @brief Construct the ZMQ publisher to send wheel telemetry data
    ///
    /// @params[in] port : Port number for the server.
    ///
    VcuTelemetryPublisher(const std::string& address, const int zmqLingerTimeInMilliSeconds, const int zmqHighWaterMarkValue,
        const VcuTelemetryPublisher::WheelsRadius& radius);

    ///
    /// @brief Copy constructor
    ///
    VcuTelemetryPublisher(const VcuTelemetryPublisher& obj) = delete;

    ///
    /// @brief Copy assignment constructor
    ///
    VcuTelemetryPublisher& operator=(VcuTelemetryPublisher other) = delete;

    ///
    /// @brief Destructor
    ///
    ~VcuTelemetryPublisher() = default;

    ///
    /// @brief Send VCUWheelEncoderTelemetry through ZMQ publisher.
    ///
    bool sendWheelTelemetry(const hal::VCUWheelID wheelId, const int timeDiffFromPrevInNanosecond, const float angle);

    ///
    /// @brief Send VCUSliderTelemetry through ZMQ publisher.
    ///
    bool sendSliderTelemetry(const hal::VCUSliderID sliderId, const float position);

    ///
    /// @brief Send VCUServoTelemetry through ZMQ publisher.
    ///
    bool sendServoTelemetry(const hal::VCUServoID servoID, const float angleInRadian);

private:
    ///
    /// @brief Get a radius of the wheel
    ///
    float getRadius(const hal::VCUWheelID id);

    zmq::context_t m_context;
    net::ZMQProtobufPublisher<hal::VCUTelemetryEnvelope> m_publisher;
    WheelsRadius m_radius;
};
} // namespace unity_plugins
