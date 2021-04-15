#pragma once

#include "packages/hal/proto/gps_telemetry.pb.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "packages/unity_plugins/gps_streamer/include/gps_reading.h"

namespace unity_plugins {

class GPSPublisher {

public:
    ///
    /// @brief Construct the GPS Server to send rendered simulated data to the client
    ///
    /// @params[in] port : Port number for the server.
    ///
    GPSPublisher(const std::string& address, const int zmqLingerTimeInMilliSeconds, const int zmqHighWaterMarkValue);

    ///
    /// @brief Copy constructor
    ///
    GPSPublisher(const GPSPublisher& obj) = delete;

    ///
    /// @brief Copy assignment constructor
    ///
    GPSPublisher& operator=(GPSPublisher other) = delete;

    ///
    /// @brief Destructor
    ///
    ~GPSPublisher() = default;

    ///
    /// @brief Send gps reading through ProtoBuffer serialization & ZMQ
    ///
    /// @params[in] gpsReading
    ///
    bool sendGPSReading(const GPSReading& gpsReading);

private:
    zmq::context_t m_context;
    net::ZMQProtobufPublisher<hal::GPSTelemetry> m_publisher;
};

} // namespace unity_plugins
