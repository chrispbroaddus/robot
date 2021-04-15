
#include "packages/hald/include/imu_device_thread.h"
#include "packages/hal/include/device_registry.h"

#include "glog/logging.h"

using namespace hald;

ImuDeviceThread::ImuDeviceThread(const Device& deviceConfig)
    : m_context(1) {
    auto publisherAddressIterator = deviceConfig.messageproperties().data().find("publisherAddress");
    if (publisherAddressIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find publisherAddress in configuration";
        throw std::runtime_error("Unable to find publisherAddress in configuration");
    }
    const std::string& publisherAddress = publisherAddressIterator->second;

    auto topicIterator = deviceConfig.messageproperties().data().find("topic");
    if (topicIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find topic in configuration";
        throw std::runtime_error("Unable to find topic in configuration");
    }
    m_topic = topicIterator->second;

    constexpr int highWaterMark = 1;
    constexpr int lingerPeriodInMilliseconds = 1000;
    m_imuSamplePublisher
        = std::unique_ptr<imu_sample_pub_t>(new imu_sample_pub_t(m_context, publisherAddress, highWaterMark, lingerPeriodInMilliseconds));

    std::map<std::string, std::string> standardMap(
        deviceConfig.deviceproperties().data().begin(), deviceConfig.deviceproperties().data().end());
    m_imu = hal::DeviceRegistry<hal::ImuDeviceInterface>::instance().create(standardMap);
    if (!m_imu.get()) {
        LOG(ERROR) << "Unable to create imu";
        throw std::runtime_error("Unable to create imu");
    }
}

ImuDeviceThread::~ImuDeviceThread() {}

void ImuDeviceThread::run() {
    while (!m_cancel) {
        hal::IMUSample imuSample;
        if (m_imu->capture(imuSample)) {
            if (!m_imuSamplePublisher->send(imuSample, m_topic)) {
                LOG(ERROR) << "Unable to send imu sample";
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
