#pragma once

#include <atomic>
#include <thread>

namespace docking {

///
/// @brief Synthetic joystick sample generator and publisher for funtionality testing
///
class SyntheticVcuTelemetryPublisher {
public:
    SyntheticVcuTelemetryPublisher(
        const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds, const std::string& topic);

    SyntheticVcuTelemetryPublisher(const SyntheticVcuTelemetryPublisher& obj) = delete;
    SyntheticVcuTelemetryPublisher& operator=(SyntheticVcuTelemetryPublisher other) = delete;

    ~SyntheticVcuTelemetryPublisher();

    void publish(const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds, const std::string& topic);

private:
    std::atomic_bool m_stop;
    std::thread m_thread;
};
}
