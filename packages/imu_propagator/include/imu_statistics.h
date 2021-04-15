
#pragma once

#include "packages/imu_propagator/include/imu_database_details.h"

namespace imu_propagator {

/// Implements a moving window statistics estimator for mean/variance.
template <typename T> class MovingStatistics {
public:
    MovingStatistics(const size_t windowSize)
        : m_samples(windowSize)
        , m_numSamplesProcessed(0)
        , m_mean(0)
        , m_variance(0)
        , m_varianceAccumulator(0) {}

    /// Push a new sample onto the window.
    /// \param x The sample to push
    void push(const T& x) {
        if (m_numSamplesProcessed == 0) {
            m_mean = x;
            m_varianceAccumulator = 0;
            m_variance = 0;
            m_samples[0] = x;
        } else if (m_numSamplesProcessed < m_samples.size()) {
            const T oldMean = m_mean;
            const T delta = x - oldMean;
            m_mean += delta / (m_numSamplesProcessed + 1);
            m_varianceAccumulator += delta * (x - m_mean);
            m_variance = m_varianceAccumulator / m_numSamplesProcessed;
            m_samples[m_numSamplesProcessed] = x;
        } else {
            const T oldMean = m_mean;
            T& oldValue = m_samples[m_numSamplesProcessed % m_samples.size()];
            const T delta = x - oldValue;
            m_mean += delta / m_samples.size();
            m_varianceAccumulator += delta * (x - m_mean + oldValue - oldMean);
            m_variance = m_varianceAccumulator / (m_samples.size() - 1);
            oldValue = x;
        }
        m_numSamplesProcessed++;
    }

    /// \return Mean value of the window
    T mean() const { return m_mean; }

    /// \return Variance of the window
    T variance() const { return m_variance; }

private:
    std::vector<T> m_samples;
    size_t m_numSamplesProcessed;
    T m_mean;
    T m_variance;
    T m_varianceAccumulator;
};

/// Estimates the moving window mean/variance of IMU samples.
template <typename TIMESTAMP, typename SCALAR> class ImuStatistics {
public:
    using imu_sample = details::ImuSample<TIMESTAMP, SCALAR>;

    ImuStatistics(const size_t windowSize)
        : m_gyroX(windowSize)
        , m_gyroY(windowSize)
        , m_gyroZ(windowSize)
        , m_accelX(windowSize)
        , m_accelY(windowSize)
        , m_accelZ(windowSize) {}
    ~ImuStatistics() = default;

    /// Update the statistics with a new sample
    /// \param sample IMU sample
    void update(const imu_sample& sample) {
        m_gyroX.push(sample.gyro()(0));
        m_gyroY.push(sample.gyro()(1));
        m_gyroZ.push(sample.gyro()(2));
        m_accelX.push(sample.accel()(0));
        m_accelY.push(sample.accel()(1));
        m_accelZ.push(sample.accel()(2));
    }

    MovingStatistics<SCALAR> gyroX() const { return m_gyroX; }
    MovingStatistics<SCALAR> gyroY() const { return m_gyroY; }
    MovingStatistics<SCALAR> gyroZ() const { return m_gyroZ; }
    MovingStatistics<SCALAR> accelX() const { return m_accelX; }
    MovingStatistics<SCALAR> accelY() const { return m_accelY; }
    MovingStatistics<SCALAR> accelZ() const { return m_accelZ; }

private:
    MovingStatistics<SCALAR> m_gyroX;
    MovingStatistics<SCALAR> m_gyroY;
    MovingStatistics<SCALAR> m_gyroZ;
    MovingStatistics<SCALAR> m_accelX;
    MovingStatistics<SCALAR> m_accelY;
    MovingStatistics<SCALAR> m_accelZ;
};
}
