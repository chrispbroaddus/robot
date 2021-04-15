
#include "packages/hald/include/gps_device_thread.h"
#include "packages/hal/include/device_registry.h"

#include "glog/logging.h"

using namespace hald;

GPSDeviceThread::GPSDeviceThread(const Device& deviceConfig)
    : m_context(1) {
    auto serverAddressIterator = deviceConfig.messageproperties().data().find("serverAddress");
    if (serverAddressIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find serverAddress in configuration";
        throw std::runtime_error("Unable to find serverAddress in configuration");
    }
    const std::string& serverAddress = serverAddressIterator->second;

    auto topicIterator = deviceConfig.messageproperties().data().find("topic");
    if (topicIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find topic in configuration";
        throw std::runtime_error("Unable to find topic in configuration");
    }
    m_topic = topicIterator->second;

    constexpr int highWaterMark = 1;
    constexpr int lingerPeriodInMilliseconds = 1000;
    m_gpsTelemetryPublisher = std::unique_ptr<gps_telemetry_pub_t>(
        new gps_telemetry_pub_t(m_context, serverAddress, highWaterMark, lingerPeriodInMilliseconds));

    std::map<std::string, std::string> standardMap(
        deviceConfig.deviceproperties().data().begin(), deviceConfig.deviceproperties().data().end());
    m_gps = hal::DeviceRegistry<hal::GPSDeviceInterface>::instance().create(standardMap);
    if (!m_gps.get()) {
        LOG(ERROR) << "Unable to create gps";
        throw std::runtime_error("Unable to create gps");
    }
}

GPSDeviceThread::~GPSDeviceThread() {}

void GPSDeviceThread::run() {
    while (!m_cancel) {
        if (m_gps->poll()) {
            hal::GPSTelemetry telemetry;
            if (m_gps->capture(telemetry)) {
                LOG(INFO) << "fix: " << telemetry.fix_mode() << " satellites: " << telemetry.num_of_satellites_in_view()
                          << " time: " << telemetry.timestamp().nanos() << " lat: " << telemetry.latitude()
                          << " lon: " << telemetry.longitude();
                if (!m_gpsTelemetryPublisher->send(telemetry, m_topic)) {
                    LOG(ERROR) << "Unable to send gps telemetry";
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}