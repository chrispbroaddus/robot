#include "packages/unity_plugins/vcu_telemetry_publisher/include/vcu_telemetry_publisher.h"
#include "packages/unity_plugins/utils/include/zippy_image_interop.h"

using namespace unity_plugins;

static std::unique_ptr<VcuTelemetryPublisher> s_publisher;

extern "C" {
ZIPPY_INTERFACE_EXPORT void VcuTelemetryPublisher_initialize(const char* address, const int zmqLingerTimeInMilliSeconds,
    const int zmqHighWaterMarkValue, const VcuTelemetryPublisher::WheelsRadius wheelsRadius) {
    s_publisher.reset(new VcuTelemetryPublisher(std::string(address), zmqLingerTimeInMilliSeconds, zmqHighWaterMarkValue, wheelsRadius));
}

ZIPPY_INTERFACE_EXPORT void VcuTelemetryPublisher_stop() {
    if (s_publisher) {
        s_publisher.reset();
    }
}

ZIPPY_INTERFACE_EXPORT void VcuTelemetryPublisher_sendWheelTelemetry(
    const hal::VCUWheelID wheelId, const int timeDiffFromPrevInNanosecond, const float distance) {
    s_publisher->sendWheelTelemetry(wheelId, timeDiffFromPrevInNanosecond, distance);
}

ZIPPY_INTERFACE_EXPORT void VcuTelemetryPublisher_sendSliderTelemetry(const hal::VCUSliderID sliderId, const float position) {
    s_publisher->sendSliderTelemetry(sliderId, position);
}

ZIPPY_INTERFACE_EXPORT void VcuTelemetryPublisher_sendServoTelemetry(const hal::VCUServoID servoId, const float angleInRadian) {
    s_publisher->sendServoTelemetry(servoId, angleInRadian);
}
}