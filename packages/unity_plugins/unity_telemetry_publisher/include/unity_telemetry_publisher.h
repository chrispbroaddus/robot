#pragma once

#include "packages/net/include/zmq_topic_pub.h"
#include "packages/unity_plugins/proto/unity_telemetry_envelope.pb.h"

#include <unordered_map>

namespace unity_plugins {

class UnityTelemetryPublisher {

public:
    ///
    /// @brief Construct the ZMQ publisher to send wheel telemetry data
    ///
    /// @params[in] port : Port number for the server.
    ///
    UnityTelemetryPublisher(const std::string& address, const int zmqLingerTimeInMilliSeconds, const int zmqHighWaterMarkValue);

    ///
    /// @brief Copy constructor
    ///
    UnityTelemetryPublisher(const UnityTelemetryPublisher& obj) = delete;

    ///
    /// @brief Copy assignment constructor
    ///
    UnityTelemetryPublisher& operator=(UnityTelemetryPublisher other) = delete;

    ///
    /// @brief Destructor
    ///
    ~UnityTelemetryPublisher() = default;

    ///
    /// @brief Send VCUWheelEncoderTelemetry through ZMQ publisher.
    ///
    bool sendGroundTruthVehiclePose(const calibration::CoordinateTransformation& pose);

private:
    zmq::context_t m_context;
    net::ZMQProtobufPublisher<unity_plugins::UnityTelemetryEnvelope> m_publisher;
};
} // namespace unity_plugins
