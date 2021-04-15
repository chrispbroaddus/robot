
#include "packages/hald/include/joystick_device_thread.h"
#include "packages/hal/include/device_registry.h"

#include "glog/logging.h"

using namespace hald;

JoystickDeviceThread::JoystickDeviceThread(const Device& deviceConfig)
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

    LOG(INFO) << "Joystick running on ZMQ address: " << serverAddress << " on topic: " << m_topic;

    constexpr int highWaterMark = 1;
    constexpr int lingerPeriodInMilliseconds = 1000;
    m_joystickSamplePublisher
        = std::unique_ptr<joystick_pub_t>(new joystick_pub_t(m_context, serverAddress, highWaterMark, lingerPeriodInMilliseconds));

    std::map<std::string, std::string> standardMap(
        deviceConfig.deviceproperties().data().begin(), deviceConfig.deviceproperties().data().end());
    m_joystick = hal::DeviceRegistry<hal::JoystickDeviceInterface>::instance().create(standardMap);
    if (!m_joystick.get()) {
        LOG(ERROR) << "Unable to create joystick";
        throw std::runtime_error("Unable to create joystick");
    }
}

JoystickDeviceThread::~JoystickDeviceThread() { stop(); }

void JoystickDeviceThread::run() {
    while (!m_cancel) {
        if (m_joystick->poll()) {
            hal::JoystickSample sample;
            if (m_joystick->capture(sample)) {
                if (!m_joystickSamplePublisher->send(sample, m_topic)) {
                    LOG(ERROR) << "Unable to send sample";
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}