#pragma once

#include <atomic>
#include <thread>

namespace docking {

///
/// @brief Synthetic joystick sample generator and publisher for funtionality testing
///
class SyntheticFiducialPosePublisher {
public:
    SyntheticFiducialPosePublisher(
        const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds, const std::string& topic);

    SyntheticFiducialPosePublisher(const SyntheticFiducialPosePublisher& obj) = delete;
    SyntheticFiducialPosePublisher& operator=(SyntheticFiducialPosePublisher other) = delete;

    ~SyntheticFiducialPosePublisher();

    void publish(const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds, const std::string& topic);

private:
    std::atomic_bool m_stop;
    std::thread m_thread;
};
}
