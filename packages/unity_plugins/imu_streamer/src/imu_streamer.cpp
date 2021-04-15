#include "imu_publisher.h"
#include "packages/unity_plugins/imu_streamer/include/imu_reading.h"
#include "packages/unity_plugins/utils/include/zippy_interface.h"
#include <cstdint>
#include <memory>
#include <string>

static std::unique_ptr<unity_plugins::IMUPublisher> s_imuPublisher;

extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API IMUStreamer_SendReading(
    const char* name, uint64_t serialNumber, unity_plugins::IMUReading reading) {
    if (s_imuPublisher) {
        s_imuPublisher->sendIMUReading(std::string(name), serialNumber, reading);
    }
}

extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API IMUStreamer_Start(
    const char* address, int zmqPubLingerTime, int zmqPubHighWaterMarkValue) {
    s_imuPublisher.reset(new unity_plugins::IMUPublisher(std::string(address), zmqPubLingerTime, zmqPubHighWaterMarkValue));
}

extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API IMUStreamer_Stop() {
    if (s_imuPublisher) {
        s_imuPublisher.reset(nullptr);
    }
}

extern "C" int ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API IMUStreamer_IsRunning() { return s_imuPublisher == nullptr ? 0 : 1; }
