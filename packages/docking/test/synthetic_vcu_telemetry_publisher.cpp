#include "synthetic_vcu_telemetry_publisher.h"
#include "glog/logging.h"

#include "packages/core/include/chrono.h"
#include "packages/hal/proto/joystick_sample.pb.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "packages/perception/fiducials/proto/fiducial_poses.pb.h"

using namespace docking;

SyntheticVcuTelemetryPublisher::SyntheticVcuTelemetryPublisher(
    const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds, const std::string& topic)
    : m_stop(false)
    , m_thread(&SyntheticVcuTelemetryPublisher::publish, this, serverAddress, highWaterMark, lingerPeriodInMilliseconds, topic) {}

SyntheticVcuTelemetryPublisher::~SyntheticVcuTelemetryPublisher() {
    m_stop = true;
    m_thread.join();
}

void SyntheticVcuTelemetryPublisher::publish(
    const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds, const std::string& topic) {

    zmq::context_t context(1);
    net::ZMQProtobufPublisher<hal::VCUTelemetryEnvelope> publisher(context, serverAddress, highWaterMark, lingerPeriodInMilliseconds);
    while (!m_stop) {
        hal::VCUTelemetryEnvelope envelope;

        const auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

        core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();

        hal::VCUServoTelemetry* servoTelemetry = new hal::VCUServoTelemetry();
        servoTelemetry->set_allocated_measurementsystemtimestamp(systemTimestamp);

        servoTelemetry->set_servoid(hal::VCUServoID::LeftRailHingeJoint);
        servoTelemetry->set_servoangleradians(M_PI / 2);

        envelope.set_allocated_servo(servoTelemetry);

        publisher.send(envelope, topic);
    }
}
