#include "packages/hald/include/network_health_device_thread.h"
#include "google/protobuf/text_format.h"
#include "packages/hal/include/device_factory.h"

#include "glog/logging.h"

#include <memory>
#include <typeinfo>

namespace {
constexpr int VERBOSE = 0;
}

namespace hald {
NetworkHealthDeviceThread::NetworkHealthDeviceThread(const Device& deviceConfig)
    : m_context(1) {

    auto addr = deviceConfig.messageproperties().data().find("serverAddress");
    auto end = deviceConfig.messageproperties().data().end();
    if (addr == end) {
        LOG(ERROR) << "Missing serverAddress in configuration";
        throw std::runtime_error("Missing serverAddress in configuration");
    }
    const auto& address = addr->second;

    auto tpc = deviceConfig.messageproperties().data().find("topic");
    if (tpc == end) {
        LOG(ERROR) << "Missing topic in configuration";
        throw std::runtime_error("Missing topic in configuration");
    }
    m_topic = tpc->second;

    try {
        constexpr int highWaterMark = 1;
        constexpr int lingerPeriodInMilliseconds = 1000;
        m_publisher.reset(new publisher_type(m_context, address, highWaterMark, lingerPeriodInMilliseconds));
    } catch (const std::exception& e) {
        LOG(ERROR) << "Caught exception when instantiating publisher: " << e.what() << " [" << typeid(e).name() << "]";
        throw e;
    } catch (...) {
        LOG(ERROR) << "Caught unhandled exception when instantiating publisher";
        throw std::runtime_error("Unhandled exception when instantiating ZMQ publisher for Network Health");
    }

    try {
        std::map<std::string, std::string> standardMap(
            deviceConfig.deviceproperties().data().begin(), deviceConfig.deviceproperties().data().end());
        m_driver = hal::DeviceRegistry<hal::NetworkHealthDriverInterface>::instance().create(standardMap);
    } catch (const std::exception& e) {
        LOG(ERROR) << "Caught exception when instantiating network health driver: " << e.what() << " [" << typeid(e).name() << "]";
        throw e;
    } catch (...) {
        LOG(ERROR) << "Caught unhandled exception when instantiating network health driver.";
        throw std::runtime_error("Unhandled exception when instantiating network health driver");
    }

    LOG(INFO) << "Network health server: [" << address << "], topic: [" << m_topic << "]";
}

NetworkHealthDeviceThread::~NetworkHealthDeviceThread() {}

void NetworkHealthDeviceThread::run() {
    LOG(INFO) << "Network health thread started";

    while (!m_cancel) {
        VLOG(VERBOSE) << "Sampling network health";

        if (0 == ::geteuid()) {
            try {
                const auto toSend = m_driver->sample();

                std::string formatted;
                ::google::protobuf::TextFormat::PrintToString(toSend, &formatted);
                LOG(INFO) << "Current network health:\n\n" << formatted;

                try {
                    VLOG(VERBOSE) << "Sending";
                    if (!m_publisher->send(toSend, m_topic)) {
                        LOG(ERROR) << "Failed to send network health status";
                    }
                } catch (const std::exception& e) {
                    LOG(ERROR) << "Failed to send network health status: [" << e.what() << "] [type: " << typeid(e).name() << "]";
                } catch (...) {
                    LOG(ERROR) << "Failed to send network health status, unhandled exception type.";
                }
            } catch (const std::exception& e) {
                LOG(ERROR) << "Failed to sample network health: [" << e.what() << "] [type: " << typeid(e).name() << "]";
            } catch (...) {
                LOG(ERROR) << "Failed to sample network health, unhandled exception type.";
            }
        } else {
            LOG(WARNING) << "Sampling network statistics requires root priveleges, which I do not currently have.";
        }

        try {
            VLOG(VERBOSE) << "Sleeping until next check-in";
            std::this_thread::sleep_for(std::chrono::seconds(30));
        } catch (...) {
            LOG(INFO) << "std::this_thread::sleep_for threw; remaining calm and carrying on.";
        }
    }

    LOG(INFO) << "Network health thread stopped";
}
}
