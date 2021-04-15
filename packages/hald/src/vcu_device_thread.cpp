
#include "packages/hald/include/vcu_device_thread.h"
#include "packages/hal/include/device_registry.h"
#include "packages/hal/include/string_utils.h"

#include "glog/logging.h"

using namespace hald;

VCUDeviceThread::VCUDeviceThread(const Device& deviceConfig)
    : m_context(1) {
    auto publisherAddressIterator = deviceConfig.messageproperties().data().find("publisherAddress");
    if (publisherAddressIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find publisherAddress in configuration";
        throw std::runtime_error("Unable to find publisherAddress in configuration");
    }
    const std::string& publisherAddress = publisherAddressIterator->second;

    auto pubTopicIterator = deviceConfig.messageproperties().data().find("publisherTopic");
    if (pubTopicIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find publisherTopic in configuration";
        throw std::runtime_error("Unable to find publisherTopic in configuration");
    }
    m_pubTopic = pubTopicIterator->second;

    auto highWaterMarkIterator = deviceConfig.messageproperties().data().find("pubHighWaterMark");
    if (highWaterMarkIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find pubHighWaterMark in configuration";
        throw std::runtime_error("Unable to find pubHighWaterMark in configuration");
    }
    uint32_t pubHighWaterMark = hal::lexicalCast<uint32_t>(highWaterMarkIterator->second);

    auto pubLingerPeriodIterator = deviceConfig.messageproperties().data().find("pubLingerPeriodInMilliseconds");
    if (pubLingerPeriodIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find pubLingerPeriodInMilliseconds in configuration";
        throw std::runtime_error("Unable to find pubLingerPeriodInMilliseconds in configuration");
    }
    uint32_t pubLingerPeriodInMilliseconds = hal::lexicalCast<uint32_t>(pubLingerPeriodIterator->second);

    m_vehicleTelemetrySamplePublisher = std::unique_ptr<telemetry_pub_t>(
        new telemetry_pub_t(m_context, publisherAddress, pubHighWaterMark, pubLingerPeriodInMilliseconds));
    LOG(INFO) << "VCU telemetry publisher running on ZMQ address: " << publisherAddress << " on topic: " << m_pubTopic;

    auto telemetryTimeoutIterator = deviceConfig.messageproperties().data().find("telemetryTimeoutInMilliseconds");
    if (telemetryTimeoutIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find telemetryTimeoutInMilliseconds in configuration";
        throw std::runtime_error("Unable to find telemetryTimeoutInMilliseconds in configuration");
    }
    m_telemetryTimeoutInMilliseconds = hal::lexicalCast<uint32_t>(telemetryTimeoutIterator->second);

    auto bindAddressIterator = deviceConfig.messageproperties().data().find("bindAddress");
    if (bindAddressIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find bindAddress in configuration";
        throw std::runtime_error("Unable to find bindAddress in configuration");
    }
    const std::string& bindAddress = bindAddressIterator->second;

    auto serverLingerPeriodIterator = deviceConfig.messageproperties().data().find("serverLingerPeriodInMilliseconds");
    if (serverLingerPeriodIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find serverLingerPeriodInMilliseconds in configuration";
        throw std::runtime_error("Unable to find serverLingerPeriodInMilliseconds in configuration");
    }
    uint32_t serverLingerPeriodInMilliseconds = hal::lexicalCast<uint32_t>(serverLingerPeriodIterator->second);

    auto serverSendRecvTimeoutIterator = deviceConfig.messageproperties().data().find("serverSendRecvTimeoutInMilliseconds");
    if (serverSendRecvTimeoutIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find serverSendRecvTimeoutInMilliseconds in configuration";
        throw std::runtime_error("Unable to find serverSendRecvTimeoutInMilliseconds in configuration");
    }
    uint32_t serverSendRecvTimeoutInMilliseconds = hal::lexicalCast<uint32_t>(serverSendRecvTimeoutIterator->second);

    m_trajectorySampleRepServer = std::unique_ptr<trajectory_rep_t>(
        new trajectory_rep_t(m_context, bindAddress, serverLingerPeriodInMilliseconds, serverSendRecvTimeoutInMilliseconds));
    LOG(INFO) << "VCU trajectory reply server running on ZMQ address: " << bindAddress;

    std::map<std::string, std::string> standardMap(
        deviceConfig.deviceproperties().data().begin(), deviceConfig.deviceproperties().data().end());
    m_vcu = hal::DeviceRegistry<hal::vcu::VCUDeviceInterface>::instance().create(standardMap);
    if (!m_vcu.get()) {
        LOG(ERROR) << "Unable to create VCU";
        throw std::runtime_error("Unable to create VCU");
    }
}

VCUDeviceThread::~VCUDeviceThread() { stop(); }

void VCUDeviceThread::run() {
    while (!m_cancel) {

        hal::VCUCommandEnvelope trajectorySample;
        hal::VCUCommandResponse trajectoryResponse;
        if (m_trajectorySampleRepServer->recv(trajectorySample)) {
            if (!m_vcu->send(trajectorySample, trajectoryResponse)) {
                LOG(ERROR) << "Unable to transmit trajectory sample";
                trajectoryResponse.set_disposition(hal::DispositionUnknown);
                if (!m_trajectorySampleRepServer->send(trajectoryResponse)) {
                    LOG(ERROR) << "Send trajectory response to planner failed: timeout";
                }
            } else {
                if (!m_trajectorySampleRepServer->send(trajectoryResponse)) {
                    LOG(ERROR) << "Send trajectory response to planner failed: timeout";
                }
            }
        }

        while (m_vcu->poll(m_telemetryTimeoutInMilliseconds)) {
            hal::VCUTelemetryEnvelope vehicleTelemetrySample;
            if (m_vcu->capture(vehicleTelemetrySample)) {
                if (!m_vehicleTelemetrySamplePublisher->send(vehicleTelemetrySample, m_pubTopic)) {
                    LOG(ERROR) << "Unable to transmit telemetry sample: timeout";
                }
            }
        }
    }
}
