#include "packages/unity_plugins/joystick_subscriber/include/joystick_subscriber.h"
#include "glog/logging.h"
#include "packages/core/include/chrono.h"
#include "packages/unity_plugins/utils/include/zippy_interface.h"

using namespace unity_plugins;

constexpr double kSecondsToNanoseconds = 1e9;

static std::unique_ptr<JoystickSubscriber> s_subscriber;

namespace {
constexpr auto JOYSTICK_TOPIC = "joystick";
}

extern "C" {
ZIPPY_INTERFACE_EXPORT int JoystickSubscriber_initialize(
    const char* addr, const int timeoutInMilliseconds, const int numJoystickAxis, const double maxLatencyInSec) {
    std::string addrStr(addr);
    s_subscriber.reset(new JoystickSubscriber(addrStr, timeoutInMilliseconds, numJoystickAxis, maxLatencyInSec));
    return 1;
}

ZIPPY_INTERFACE_EXPORT JoystickSubscriber::JoystickAxisResponse JoystickSubscriber_receive() {
    JoystickSubscriber::JoystickAxisResponse response;
    response.success = 0;
    hal::JoystickSample sample;
    if (s_subscriber && s_subscriber->receive(sample)) {
        response.success = 1;
        response.axis0 = sample.axis(0);
        response.axis1 = sample.axis(1);
    }
    return response;
}
}

JoystickSubscriber::JoystickSubscriber(const std::string& addr, const int highWaterMark, const int axisSize, const double maxLatencyInSec)
    : m_context(1)
    , m_maxLatencyInSec(maxLatencyInSec)
    , m_subscriber(m_context, addr, JOYSTICK_TOPIC, highWaterMark) {}

bool JoystickSubscriber::receive(hal::JoystickSample& sample) {
    if (m_subscriber.poll(std::chrono::milliseconds(500)) && m_subscriber.recv(sample)) {
        CHECK(sample.axis_size() >= 2);
        CHECK((uint64_t)core::chrono::gps::wallClockInNanoseconds().count() >= sample.systemtimestamp().nanos());
        return true;
    } else {
        return false;
    }
}
