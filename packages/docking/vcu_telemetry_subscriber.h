#pragma once

#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "vcu_ik_telemetry_state.h"
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

namespace docking {

/// Subscribing telemetry data about hinge heights and pitch-angles for IK control
class VcuTelemetrySubscriber {

public:
    ///
    /// \param addr   ZMQ subscriber address
    /// \param topic  ZMQ topic
    ///
    VcuTelemetrySubscriber(const std::string& addr, const std::string& topic);

    ~VcuTelemetrySubscriber();

    ///
    /// \brief Read the telemetry data where data is expected to be with small delay within the latest time window
    /// \param telemetryState     Output telemetry state
    /// \param timeWindowInNanos  The time window in nano seconds
    /// \return true if there exist data
    bool readTelemetry(VcuIkTelemetryState& telemetryState, const uint64_t timeWindowInNanos);

private:
    ///
    /// \brief Listener running on a thread to read the telemetry data from ZMQ publisher
    /// \param addr   ZMQ address
    /// \param topic  ZMQ topic
    /// \param stop   Controlling variable for a thread
    void listen(const std::string& addr, const std::string& topic, std::atomic_bool& stop);

    std::atomic_bool m_stop;
    std::thread m_thread;
    std::mutex m_mutex;
    VcuIkTelemetryState m_telemetryState;
};
}
