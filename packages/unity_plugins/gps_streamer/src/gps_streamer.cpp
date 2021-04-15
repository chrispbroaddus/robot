#include "gps_publisher.h"
#include "packages/unity_plugins/gps_streamer/include/gps_reading.h"
#include "packages/unity_plugins/utils/include/zippy_interface.h"
#include <memory>
#include <string>

static std::unique_ptr<unity_plugins::GPSPublisher> s_gpsPublisher;

extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API GPSStreamer_SendReading(unity_plugins::GPSReading reading) {
    if (s_gpsPublisher) {
        s_gpsPublisher->sendGPSReading(reading);
    }
}

extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API GPSStreamer_Start(
    const char* address, int zmqPubLingerTime, int zmqPubHighWaterMarkValue) {
    s_gpsPublisher.reset(new unity_plugins::GPSPublisher(std::string(address), zmqPubLingerTime, zmqPubHighWaterMarkValue));
}

extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API GPSStreamer_Stop() {
    if (s_gpsPublisher) {
        s_gpsPublisher.reset(nullptr);
    }
}

extern "C" int ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API GPSStreamer_IsRunning() { return s_gpsPublisher == nullptr ? 0 : 1; }