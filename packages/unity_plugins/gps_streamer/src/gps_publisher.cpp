#include "gps_publisher.h"
#include "packages/core/include/chrono.h"
#include "packages/core/proto/timestamp.pb.h"
#include "packages/hal/proto/gps_telemetry.pb.h"

#include <chrono>
#include <limits>

using namespace unity_plugins;

namespace {
constexpr auto GPS_TOPIC = "gps";
}

GPSPublisher::GPSPublisher(const std::string& address, const int zmqLingerTimeInMilliSeconds, const int zmqHighWaterMarkValue)
    : m_context(1)
    , m_publisher(m_context, address, zmqHighWaterMarkValue, zmqLingerTimeInMilliSeconds) {}

bool GPSPublisher::sendGPSReading(const GPSReading& gpsReading) {

    hal::GPSTelemetry gpsTelemetry;

    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

    core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
    systemTimestamp->set_nanos(gpsTimestamp.count());

    gpsTelemetry.set_allocated_timestamp(systemTimestamp);

    gpsTelemetry.set_latitude(gpsReading.latitude);
    gpsTelemetry.set_longitude(gpsReading.longitude);
    gpsTelemetry.set_altitude(gpsReading.altitude);

    gpsTelemetry.set_horizontal_dilution_of_precision(0);
    gpsTelemetry.set_num_of_satellites_in_view(std::numeric_limits<uint32_t>::max());

    bool sendSuccess = m_publisher.send(gpsTelemetry, GPS_TOPIC);

    return sendSuccess;
}
