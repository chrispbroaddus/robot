#include "packages/unity_plugins/simulated_cameras/include/simulator_stats_publisher.h"
#include "glog/logging.h"
#include "packages/unity_plugins/simulated_cameras/include/simulator_publisher_topics.h"

using namespace unity_plugins;

SimulatorStatsPublisher::SimulatorStatsPublisher(
    std::shared_ptr<zmq::context_t> context, const std::string& address, int zmqLingerTimeInMilliSeconds, int zmqHighWaterMarkValue)
    : m_context(context)
    , m_statsPublisher(*m_context, address, zmqHighWaterMarkValue, zmqLingerTimeInMilliSeconds) {}

void SimulatorStatsPublisher::sendStats(const SimulatorStats& stats) {

    hal::SimulatorStatsTelemetry statsTelemetry;

    statsTelemetry.set_deltatimems(stats.deltaTimeMs);
    statsTelemetry.set_framenumber(stats.frameNumber);

    bool sendSuccess = m_statsPublisher.send(statsTelemetry, STATS_TOPIC);
    CHECK(sendSuccess);
}
