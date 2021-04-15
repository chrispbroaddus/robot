#include "packages/hal/include/drivers/imus/unity_simulated/unity_simulated_imu_device.h"
#include "glog/logging.h"
#include "packages/hal/include/string_utils.h"

#include <chrono>

namespace hal {

UnitySimulatedImuDevice::UnitySimulatedImuDevice(const uint32_t id, const details::property_map_t& config)
    : m_imuId(id) {

    auto publisherAddressIterator = config.find("publisherAddress");
    if (publisherAddressIterator == config.end()) {
        LOG(ERROR) << "Unable to find publisherAddress in configuration";
        throw std::runtime_error("Unable to find publisherAddress in configuration");
    }
    std::string publisherAddress = publisherAddressIterator->second;

    auto topicIterator = config.find("topic");
    if (topicIterator == config.end()) {
        LOG(ERROR) << "Unable to find topic in configuration";
        throw std::runtime_error("Unable to find topic in configuration");
    }
    std::string topic = topicIterator->second;

    auto highWaterMarkIterator = config.find("subscriberHighWaterMark");
    if (highWaterMarkIterator == config.end()) {
        LOG(ERROR) << "Unable to find subscriberHighWaterMark in configuration";
        throw std::runtime_error("Unable to find subscriberHighWaterMark in configuration");
    }
    int subscriberHighWaterMark = lexicalCast<int>(highWaterMarkIterator->second);

    auto captureTimeoutIterator = config.find("captureTimeoutInMilliseconds");
    if (captureTimeoutIterator == config.end()) {
        LOG(ERROR) << "Unable to find captureTimeoutInMilliseconds in configuration";
        throw std::runtime_error("Unable to find captureTimeoutInMilliseconds in configuration");
    }
    m_captureTimeoutInMilliseconds = lexicalCast<int>(captureTimeoutIterator->second);

    m_imuSampleSubscriber
        = std::unique_ptr<imu_sample_sub_t>(new imu_sample_sub_t(m_context, publisherAddress, topic, subscriberHighWaterMark));
    LOG(INFO) << "Unity IMU subscriber running on ZMQ address: " << publisherAddress << " on topic: " << topic;

    LOG(INFO) << "Unity Simulated IMU created";
}

bool UnitySimulatedImuDevice::capture(IMUSample& imuSample) {

    LOG(INFO) << "Capturing IMU sample";
    if (m_imuSampleSubscriber->poll(std::chrono::milliseconds(m_captureTimeoutInMilliseconds))) {
        if (!m_imuSampleSubscriber->recv(imuSample)) {
            LOG(ERROR) << "Capture IMU sample failed";
            return false;
        }
    } else {
        LOG(ERROR) << "Capture IMU sample timed out";
        return false;
    }
    LOG_EVERY_N(INFO, 100) << "Captured IMU sample";

    return true;
}
}
