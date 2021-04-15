#include "packages/hal/include/drivers/vcu/unity_simulated/unity_simulated_vcu.h"
#include "glog/logging.h"
#include "packages/hal/include/string_utils.h"

namespace hal {

namespace vcu {

    UnitySimulatedVcu::UnitySimulatedVcu(const hal::details::property_map_t& config)
        : m_context(1) {
        auto serverAddressIterator = config.find("repServerAddress");
        if (serverAddressIterator == config.end()) {
            LOG(ERROR) << "Unable to find repServerAddress in configuration";
            throw std::runtime_error("Unable to find repServerAddress in configuration");
        }
        const std::string& repServerAddress = serverAddressIterator->second;

        auto lingerPeriodIterator = config.find("lingerPeriodInMilliseconds");
        if (lingerPeriodIterator == config.end()) {
            LOG(ERROR) << "Unable to find lingerPeriodInMilliseconds in configuration";
            throw std::runtime_error("Unable to find lingerPeriodInMilliseconds in configuration");
        }
        int lingerPeriodInMilliseconds = lexicalCast<int>(lingerPeriodIterator->second);

        auto sendRecvTimeoutIterator = config.find("sendRecvTimeoutInMilliseconds");
        if (sendRecvTimeoutIterator == config.end()) {
            LOG(ERROR) << "Unable to find sendRecvTimeoutInMilliseconds in configuration";
            throw std::runtime_error("Unable to find sendRecvTimeoutInMilliseconds in configuration");
        }
        int sendRecvTimeoutInMilliseconds = lexicalCast<int>(sendRecvTimeoutIterator->second);

        m_CommandReqClient = std::unique_ptr<command_req_t>(
            new command_req_t(m_context, repServerAddress, lingerPeriodInMilliseconds, sendRecvTimeoutInMilliseconds));
        LOG(INFO) << "VCU trajectory request client running on ZMQ address: " << repServerAddress;

        auto subscriberAddressIterator = config.find("subscriberAddress");
        if (subscriberAddressIterator == config.end()) {
            LOG(ERROR) << "Unable to find subscriberAddress in configuration";
            throw std::runtime_error("Unable to find subscriberAddress in configuration");
        }
        const std::string& subscriberAddress = subscriberAddressIterator->second;

        auto subTopicIterator = config.find("subscriberTopic");
        if (subTopicIterator == config.end()) {
            LOG(ERROR) << "Unable to find subscriberTopic in configuration";
            throw std::runtime_error("Unable to find subscriberTopic in configuration");
        }
        m_subTopic = subTopicIterator->second;

        auto highWaterMarkIterator = config.find("subscriberHighWaterMark");
        if (highWaterMarkIterator == config.end()) {
            LOG(ERROR) << "Unable to find subscriberHighWaterMark in configuration";
            throw std::runtime_error("Unable to find subscriberHighWaterMark in configuration");
        }
        int subscriberHighWaterMark = lexicalCast<int>(highWaterMarkIterator->second);

        m_vehicleTelemetrySubscriber
            = std::unique_ptr<telemetry_sub_t>(new telemetry_sub_t(m_context, subscriberAddress, m_subTopic, subscriberHighWaterMark));
        CHECK_NOTNULL(m_vehicleTelemetrySubscriber.get());
        LOG(INFO) << "VCU telemetry subscriber running on ZMQ address: " << subscriberAddress << " on topic: " << m_subTopic;

        LOG(INFO) << "Created Unity Simulated VCU device";
    }

    bool UnitySimulatedVcu::poll(uint32_t timeoutInMilliseconds) {
        return m_vehicleTelemetrySubscriber->poll(std::chrono::milliseconds(timeoutInMilliseconds));
    }

    bool UnitySimulatedVcu::capture(VCUTelemetryEnvelope& telemetryEnvelope) {
        LOG(INFO) << "Capturing telemetry sample";

        if (!m_vehicleTelemetrySubscriber->recv(telemetryEnvelope)) {
            LOG(ERROR) << "Receive telemetry sample from VCU failed: timeout";
            return false;
        }

        return true;
    }

    bool UnitySimulatedVcu::send(const VCUCommandEnvelope& commandEnvelope, VCUCommandResponse& commandResponse) {
        LOG(INFO) << "Sending trajectory sample";
        if (!m_CommandReqClient->send(commandEnvelope)) {
            LOG(ERROR) << "Send trajectory sample to VCU failed: timeout";
            return false;
        }

        if (!m_CommandReqClient->recv(commandResponse)) {
            LOG(ERROR) << "Receive trajectory response from VCU failed: timeout";
            return false;
        }

        return true;
    }

    UnitySimulatedVcu::~UnitySimulatedVcu() { LOG(INFO) << "Destroyed Unity Simulated VCU device"; }

} // vcu
} // hal
