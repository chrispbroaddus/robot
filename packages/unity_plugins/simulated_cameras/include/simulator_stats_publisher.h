#pragma once

#include "packages/hal/proto/simulator_stats_telemetry.pb.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "simulator_stats.h"
#include <memory>

namespace unity_plugins {

class SimulatorStatsPublisher {

public:
    ///
    /// @brief Construct the Stats Publisher to send simulator stats to the client
    ///
    /// @params[in] context : Shared zmq context
    /// @params[in] port : Port number for the server.
    /// @params[in] zmqLingerTimeInMilliSeconds : How long to linger for in millisecons
    /// @params[in] zmqHighWaterMarkValue : High water mark value (number of queued messages to keep before discarding)
    ///
    SimulatorStatsPublisher(
        std::shared_ptr<zmq::context_t> context, const std::string& address, int zmqLingerTimeInMilliSeconds, int zmqHighWaterMarkValue);

    ///
    /// @brief Copy constructor
    ///
    SimulatorStatsPublisher(const SimulatorStatsPublisher& obj) = delete;

    ///
    /// @brief Copy assignment constructor
    ///
    SimulatorStatsPublisher& operator=(SimulatorStatsPublisher other) = delete;

    ///
    /// @brief Destructor
    ///
    ~SimulatorStatsPublisher() = default;

    ///
    /// @brief Send stats data through ProtoBuffer serialization & ZMQ
    ///
    /// @params[in] stats
    ///
    void sendStats(const SimulatorStats& stats);

private:
    std::shared_ptr<zmq::context_t> m_context;
    net::ZMQProtobufPublisher<hal::SimulatorStatsTelemetry> m_statsPublisher;
};
} // namespace unity_plugins
