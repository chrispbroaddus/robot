#pragma once

#include <atomic>
#include <thread>

namespace unity_plugins {

///
/// @brief Synthetic joystick sample generator and publisher for funtionality testing
///
class SyntheticJoystickPublisher {
public:
    SyntheticJoystickPublisher(const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds);

    SyntheticJoystickPublisher(const SyntheticJoystickPublisher& obj) = delete;
    SyntheticJoystickPublisher& operator=(SyntheticJoystickPublisher other) = delete;

    ~SyntheticJoystickPublisher();

    void publish(const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds);

private:
    std::atomic_bool m_stop;
    std::thread m_thread;
};
} // namespace unity_plugins