#include "packages/unity_plugins/unity_telemetry_publisher/include/unity_telemetry_publisher.h"
#include "glog/logging.h"
#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "packages/core/include/chrono.h"
#include "packages/unity_plugins/proto/ground_truth_vehicle_pose.pb.h"

#include <chrono>

using namespace unity_plugins;

namespace {
constexpr auto TELEMETRY_TOPIC = "telemetry";
}

UnityTelemetryPublisher::UnityTelemetryPublisher(
    const std::string& address, const int zmqLingerTimeInMilliSeconds, const int zmqHighWaterMarkValue)
    : m_context(1)
    , m_publisher(m_context, address, zmqHighWaterMarkValue, zmqLingerTimeInMilliSeconds) {}

bool UnityTelemetryPublisher::sendGroundTruthVehiclePose(const calibration::CoordinateTransformation& pose) {

    unity_plugins::UnityTelemetryEnvelope* envelope = new unity_plugins::UnityTelemetryEnvelope();
    CHECK_NOTNULL(envelope);

    unity_plugins::GroundTruthVehiclePose* poseMessage = new unity_plugins::GroundTruthVehiclePose();
    *poseMessage->mutable_transformations() = pose;

    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

    core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
    systemTimestamp->set_nanos(gpsTimestamp.count());
    poseMessage->set_allocated_measurementsystemtimestamp(systemTimestamp);

    envelope->set_allocated_vehiclepose(poseMessage);

    return m_publisher.send(*envelope, TELEMETRY_TOPIC);
}
