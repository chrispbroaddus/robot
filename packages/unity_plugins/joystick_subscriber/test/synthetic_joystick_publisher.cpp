#include "synthetic_joystick_publisher.h"
#include "glog/logging.h"

#include "packages/core/include/chrono.h"
#include "packages/hal/proto/joystick_sample.pb.h"
#include "packages/net/include/zmq_topic_pub.h"

using namespace unity_plugins;

SyntheticJoystickPublisher::SyntheticJoystickPublisher(
    const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds)
    : m_stop(false)
    , m_thread(&SyntheticJoystickPublisher::publish, this, serverAddress, highWaterMark, lingerPeriodInMilliseconds) {}

SyntheticJoystickPublisher::~SyntheticJoystickPublisher() {
    m_stop = true;
    m_thread.join();
}

void SyntheticJoystickPublisher::publish(const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds) {

    zmq::context_t context(1);
    net::ZMQProtobufPublisher<hal::JoystickSample> publisher(context, serverAddress, highWaterMark, lingerPeriodInMilliseconds);

    while (!m_stop) {
        hal::JoystickSample joystickSample;

        const auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();
        joystickSample.mutable_systemtimestamp()->set_nanos(gpsTimestamp.count());
        joystickSample.add_axis(-1.f);
        joystickSample.add_axis(+1.f);
        publisher.send(joystickSample, "joystick");
    }
}
