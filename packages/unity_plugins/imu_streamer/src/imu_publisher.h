#pragma once

#include "packages/hal/proto/imu_sample.pb.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "packages/unity_plugins/imu_streamer/include/imu_reading.h"

#include <cstdint>

namespace unity_plugins {

class IMUPublisher {

public:
    ///
    /// @brief Construct the IMU Server to send simulated imu data to the client
    ///
    /// @params[in] address : server address.
    ///
    IMUPublisher(const std::string& address, const int zmqLingerTimeInMilliSeconds, const int zmqHighWaterMarkValue);

    ///
    /// @brief Copy constructor
    ///
    IMUPublisher(const IMUPublisher& obj) = delete;

    ///
    /// @brief Copy assignment constructor
    ///
    IMUPublisher& operator=(IMUPublisher other) = delete;

    ///
    /// @brief Destructor
    ///
    ~IMUPublisher() = default;

    ///
    /// @brief Send imu reading through ProtoBuffer serialization & ZMQ
    ///
    /// @params[in] imuReading
    ///
    bool sendIMUReading(const std::string& name, uint64_t serialNumber, const IMUReading& imuReading);

private:
    zmq::context_t m_context;
    net::ZMQProtobufPublisher<hal::IMUSample> m_publisher;
};

} // namespace unity_plugins
