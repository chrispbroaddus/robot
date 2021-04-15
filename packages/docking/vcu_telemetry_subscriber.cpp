#include "vcu_telemetry_subscriber.h"
#include "glog/logging.h"
#include "packages/core/include/chrono.h"

namespace docking {

VcuTelemetrySubscriber::VcuTelemetrySubscriber(const std::string& addr, const std::string& topic)
    : m_stop(false)
    , m_thread(&VcuTelemetrySubscriber::listen, this, addr, topic, std::ref(m_stop)) {}

VcuTelemetrySubscriber::~VcuTelemetrySubscriber() {
    m_stop = true;
    m_thread.join();
}

bool VcuTelemetrySubscriber::readTelemetry(VcuIkTelemetryState& telemetryState, const uint64_t timeWindowInNanos) {
    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds().count();

    std::lock_guard<std::mutex> guard(m_mutex);

    if (gpsTimestamp - m_telemetryState.timestampLeftServoAngle < timeWindowInNanos
        && gpsTimestamp - m_telemetryState.timestampLeftServoHeight < timeWindowInNanos
        && gpsTimestamp - m_telemetryState.timestampRightServoAngle < timeWindowInNanos
        && gpsTimestamp - m_telemetryState.timestampRightServoHeight < timeWindowInNanos) {

        telemetryState.leftServoAngle = m_telemetryState.leftServoAngle;
        telemetryState.timestampLeftServoAngle = m_telemetryState.timestampLeftServoAngle;
        telemetryState.rightServoAngle = m_telemetryState.rightServoAngle;
        telemetryState.timestampRightServoAngle = m_telemetryState.timestampRightServoAngle;
        telemetryState.leftServoHeight = m_telemetryState.leftServoHeight;
        telemetryState.timestampLeftServoHeight = m_telemetryState.timestampLeftServoHeight;
        telemetryState.rightServoHeight = m_telemetryState.rightServoHeight;
        telemetryState.timestampRightServoHeight = m_telemetryState.timestampRightServoHeight;

        return true;
    } else {
        VLOG(1) << __PRETTY_FUNCTION__ << " ... telemetry is outdated.";
        VLOG(2) << __PRETTY_FUNCTION__ << " ... ... gpsTimestamp : " << gpsTimestamp;
        VLOG(2) << __PRETTY_FUNCTION__
                << " ... ... m_telemetryState.timestampLeftServoAngle : " << m_telemetryState.timestampLeftServoAngle;
        VLOG(2) << __PRETTY_FUNCTION__
                << " ... ... m_telemetryState.timestampLeftServoHeight : " << m_telemetryState.timestampLeftServoHeight;
        VLOG(2) << __PRETTY_FUNCTION__
                << " ... ... m_telemetryState.timestampRightServoAngle : " << m_telemetryState.timestampRightServoAngle;
        VLOG(2) << __PRETTY_FUNCTION__
                << " ... ... m_telemetryState.timestampRightServoHeight : " << m_telemetryState.timestampRightServoHeight;
        VLOG(2) << __PRETTY_FUNCTION__ << " ... ... timeWindowInNanos : " << timeWindowInNanos;
        return false;
    }
}

void VcuTelemetrySubscriber::listen(const std::string& addr, const std::string& topic, std::atomic_bool& stop) {

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<hal::VCUTelemetryEnvelope> subscriber(context, addr, topic, 1);

    while (!stop) {
        if (subscriber.poll(std::chrono::milliseconds(1))) {
            hal::VCUTelemetryEnvelope envelope;
            if (subscriber.recv(envelope)) {
                if (envelope.has_wheelencoder()) {
                    auto wheelTelemetry = envelope.wheelencoder();
                    VLOG(2) << __FUNCTION__ << " ... Wheel ID : " << wheelTelemetry.wheelid();
                    VLOG(2) << __FUNCTION__ << " ... ... Dist : " << wheelTelemetry.lineardisplacementmeters();
                    VLOG(2) << __FUNCTION__ << " ... ... Timestamp : " << wheelTelemetry.periodendsystemtimestamp().nanos();
                }
                if (envelope.has_slider()) {
                    auto slider = envelope.slider();
                    VLOG(2) << __FUNCTION__ << " ... Slider ID : " << slider.sliderid();
                    VLOG(2) << __FUNCTION__ << " ... ... position : " << slider.positioninmeters();
                    VLOG(2) << __FUNCTION__ << " ... ... Timestamp : " << slider.measurementsystemtimestamp().nanos();
                    if (slider.sliderid() == hal::VCUSliderID::LeftRailSlider) {
                        std::lock_guard<std::mutex> guard(m_mutex);
                        m_telemetryState.leftServoHeight = slider.positioninmeters();
                        m_telemetryState.timestampLeftServoHeight = slider.measurementsystemtimestamp().nanos();
                    } else if (slider.sliderid() == hal::VCUSliderID::RightRailSlider) {
                        std::lock_guard<std::mutex> guard(m_mutex);
                        m_telemetryState.rightServoHeight = slider.positioninmeters();
                        m_telemetryState.timestampRightServoHeight = slider.measurementsystemtimestamp().nanos();
                    }
                }
                if (envelope.has_servo()) {
                    auto servo = envelope.servo();
                    VLOG(2) << __FUNCTION__ << " ... Servo ID : " << servo.servoid();
                    VLOG(2) << __FUNCTION__ << " ... ... angle(radian) : " << servo.servoangleradians();
                    VLOG(2) << __FUNCTION__ << " ... ... Timestamp : " << servo.measurementsystemtimestamp().nanos();
                    if (servo.servoid() == hal::VCUServoID::LeftRailHingeJoint) {
                        std::lock_guard<std::mutex> guard(m_mutex);
                        m_telemetryState.leftServoAngle = servo.servoangleradians();
                        m_telemetryState.timestampLeftServoAngle = servo.measurementsystemtimestamp().nanos();
                    } else if (servo.servoid() == hal::VCUServoID::RightRailHingeJoint) {
                        std::lock_guard<std::mutex> guard(m_mutex);
                        m_telemetryState.rightServoAngle = servo.servoangleradians();
                        m_telemetryState.timestampRightServoAngle = servo.measurementsystemtimestamp().nanos();
                    }
                }

            } // end of if (subscriber.recv(envelope))

        } // if (subscriber.poll(std::chrono::microseconds(10)))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
}
