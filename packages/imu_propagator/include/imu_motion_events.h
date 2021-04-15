
#pragma once

#include "packages/imu_propagator/include/imu_statistics.h"
#include <stdexcept>

namespace imu_propagator {

/// Implements a simple motion event detector based on heuristics on the gyroscope and accelerometer
/// mean and variance in a sliding window.
template <typename TIMESTAMP, typename SCALAR> class ImuMotionEvents {
public:
    using imu_sample = details::ImuSample<TIMESTAMP, SCALAR>;
    using imu_statistics = ImuStatistics<TIMESTAMP, SCALAR>;

    ImuMotionEvents(const size_t windowSize, const SCALAR gyroMeanThreshold, const SCALAR gyroStandardDeviationThreshold,
        const SCALAR accelMeanThreshold, const SCALAR accelStandardDeviationThreshold, const SCALAR gravityInMetersPerSecond)
        : m_imuStatistics(windowSize)
        , m_gyroMeanThreshold(gyroMeanThreshold)
        , m_gyroStandardDeviationThreshold(gyroStandardDeviationThreshold)
        , m_accelMeanThreshold(accelMeanThreshold)
        , m_accelStandardDeviationThreshold(accelStandardDeviationThreshold)
        , m_gravityInMetersPerSecond(gravityInMetersPerSecond)
        , m_isStationary(false) {
        constexpr SCALAR minGravityInMetersPerSecond = 9;
        constexpr SCALAR maxGravityInMetersPerSecond = 11;
        if (gravityInMetersPerSecond < minGravityInMetersPerSecond || gravityInMetersPerSecond > maxGravityInMetersPerSecond) {
            throw std::out_of_range("gravity in meters per second is out of range");
        }
    }
    ~ImuMotionEvents() = default;

    /// Update the detector with a new sample
    /// \param sample IMU sample
    void update(const imu_sample& sample) {
        m_isStationary = false;

        m_imuStatistics.update(sample);

        const SCALAR gyroStandardDeviationThresholdSquared = m_gyroStandardDeviationThreshold * m_gyroStandardDeviationThreshold;
        const SCALAR accelStandardDeviationThresholdSquared = m_accelStandardDeviationThreshold * m_accelStandardDeviationThreshold;

        bool gyroInZeroEnvelope = false;
        if (std::abs(m_imuStatistics.gyroX().mean()) < m_gyroMeanThreshold && std::abs(m_imuStatistics.gyroY().mean()) < m_gyroMeanThreshold
            && std::abs(m_imuStatistics.gyroZ().mean()) < m_gyroMeanThreshold
            && m_imuStatistics.gyroX().variance() < gyroStandardDeviationThresholdSquared
            && m_imuStatistics.gyroY().variance() < gyroStandardDeviationThresholdSquared
            && m_imuStatistics.gyroZ().variance() < gyroStandardDeviationThresholdSquared) {
            gyroInZeroEnvelope = true;
        }

        bool accelInZeroEnvelope = false;
        const SCALAR accelMagnitude = std::sqrt(m_imuStatistics.accelX().mean() * m_imuStatistics.accelX().mean()
            + m_imuStatistics.accelY().mean() * m_imuStatistics.accelY().mean()
            + m_imuStatistics.accelZ().mean() * m_imuStatistics.accelZ().mean());
        if (std::abs(accelMagnitude - m_gravityInMetersPerSecond) < m_accelMeanThreshold
            && m_imuStatistics.accelX().variance() < accelStandardDeviationThresholdSquared
            && m_imuStatistics.accelY().variance() < accelStandardDeviationThresholdSquared
            && m_imuStatistics.accelZ().variance() < accelStandardDeviationThresholdSquared) {
            accelInZeroEnvelope = true;
        }

        if (gyroInZeroEnvelope && accelInZeroEnvelope) {
            m_isStationary = true;
        }
    }

    /// \return True if the IMU is stationary, otherwise false
    bool isStationary() const { return m_isStationary; }

private:
    imu_statistics m_imuStatistics;
    const SCALAR m_gyroMeanThreshold;
    const SCALAR m_gyroStandardDeviationThreshold;
    const SCALAR m_accelMeanThreshold;
    const SCALAR m_accelStandardDeviationThreshold;
    const SCALAR m_gravityInMetersPerSecond;
    bool m_isStationary;
};
}