#include "packages/unity_plugins/vcu_telemetry_publisher/include/vcu_telemetry_publisher.h"
#include "glog/logging.h"
#include "packages/core/include/chrono.h"

#include <chrono>

namespace {
constexpr auto TELEMETRY_TOPIC = "telemetry";
}

namespace unity_plugins {

VcuTelemetryPublisher::VcuTelemetryPublisher(const std::string& address, const int zmqLingerTimeInMilliSeconds,
    const int zmqHighWaterMarkValue, const VcuTelemetryPublisher::WheelsRadius& radius)
    : m_context(1)
    , m_publisher(m_context, address, zmqHighWaterMarkValue, zmqLingerTimeInMilliSeconds)
    , m_radius(radius) {}

bool VcuTelemetryPublisher::sendWheelTelemetry(
    const hal::VCUWheelID wheelId, const int timeDiffFromPrevInNanosecond, const float distance) {

    hal::VCUWheelEncoderTelemetry* telemetry = new hal::VCUWheelEncoderTelemetry();

    telemetry->set_wheelid(wheelId);

    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

    core::HardwareTimestamp* endHardwareTimestamp = new core::HardwareTimestamp();
    endHardwareTimestamp->set_nanos(gpsTimestamp.count());
    telemetry->set_allocated_periodendhardwaretimestamp(endHardwareTimestamp);

    core::HardwareTimestamp* startHardwareTimestamp = new core::HardwareTimestamp();
    startHardwareTimestamp->set_nanos(gpsTimestamp.count() - timeDiffFromPrevInNanosecond);
    telemetry->set_allocated_periodstarthardwaretimestamp(startHardwareTimestamp);

    core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
    systemTimestamp->set_nanos(gpsTimestamp.count());
    telemetry->set_allocated_periodendsystemtimestamp(systemTimestamp);

    telemetry->set_lineardisplacementmeters(distance);

    hal::VCUTelemetryEnvelope envelope;
    envelope.mutable_sendtimestamp()->set_nanos(gpsTimestamp.count());
    envelope.set_allocated_wheelencoder(telemetry);

    return m_publisher.send(envelope, TELEMETRY_TOPIC);
}

bool VcuTelemetryPublisher::sendSliderTelemetry(const hal::VCUSliderID sliderId, const float position) {

    hal::VCUSliderTelemetry* telemetry = new hal::VCUSliderTelemetry();

    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

    core::HardwareTimestamp* hardwareTimestamp = new core::HardwareTimestamp();
    hardwareTimestamp->set_nanos(gpsTimestamp.count());
    telemetry->set_allocated_measurementhardwaretimestamp(hardwareTimestamp);

    core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
    systemTimestamp->set_nanos(gpsTimestamp.count());
    telemetry->set_allocated_measurementsystemtimestamp(systemTimestamp);

    telemetry->set_positioninmeters(position);
    telemetry->set_sliderid(sliderId);

    hal::VCUTelemetryEnvelope envelope;
    envelope.mutable_sendtimestamp()->set_nanos(gpsTimestamp.count());
    envelope.set_allocated_slider(telemetry);

    return m_publisher.send(envelope, TELEMETRY_TOPIC);
}

bool VcuTelemetryPublisher::sendServoTelemetry(const hal::VCUServoID servoID, const float angleInRadian) {

    hal::VCUServoTelemetry* telemetry = new hal::VCUServoTelemetry();

    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

    core::HardwareTimestamp* hardwareTimestamp = new core::HardwareTimestamp();
    hardwareTimestamp->set_nanos(gpsTimestamp.count());
    telemetry->set_allocated_measurementhardwaretimestamp(hardwareTimestamp);

    core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
    systemTimestamp->set_nanos(gpsTimestamp.count());
    telemetry->set_allocated_measurementsystemtimestamp(systemTimestamp);

    telemetry->set_servoangleradians(angleInRadian);
    telemetry->set_servoid(servoID);

    hal::VCUTelemetryEnvelope envelope;
    envelope.mutable_sendtimestamp()->set_nanos(gpsTimestamp.count());
    envelope.set_allocated_servo(telemetry);

    return m_publisher.send(envelope, TELEMETRY_TOPIC);
}

} // namespace unity_plugins
