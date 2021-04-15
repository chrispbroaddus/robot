
#pragma once

#include "packages/imu_propagator/include/imu_database_details.h"
#include <vector>

namespace imu_propagator {

/// Estimates the white noise sigma and bias sigma in the continuous time domain.
///     xhat = x + wx + b where wx ~ N(0, sx^2)
///     b(k+1) = b(k) + wb where wb ~ N(0, sb^2)
///
/// https://github.com/ethz-asl/kalibr/wiki/IMU-Noise-Model
class ImuParameterEstimation {
public:
    using noise_type = Eigen::Matrix<double, 3, 1>;
    using imu_sample_type = details::ImuSample<double, double>;

    static constexpr size_t kDefaultMaxChunkSize = 1000;

    ImuParameterEstimation(const std::vector<imu_sample_type>& samples, const size_t maxChunkSize);
    ~ImuParameterEstimation() = default;

    const noise_type& gyroWhiteNoiseSigma() const { return m_gyroWhiteNoiseSigma; }
    const noise_type& gyroBiasSigma() const { return m_gyroBiasSigma; }
    const noise_type& accelWhiteNoiseSigma() const { return m_accelWhiteNoiseSigma; }
    const noise_type& accelBiasSigma() const { return m_accelBiasSigma; }

private:
    const size_t m_maxChunkSize;
    noise_type m_gyroWhiteNoiseSigma;
    noise_type m_gyroBiasSigma;
    noise_type m_accelWhiteNoiseSigma;
    noise_type m_accelBiasSigma;
    double m_meanDt;

    std::vector<double> m_time;
    std::vector<double> m_gyroX;
    std::vector<double> m_gyroY;
    std::vector<double> m_gyroZ;
    std::vector<double> m_accelX;
    std::vector<double> m_accelY;
    std::vector<double> m_accelZ;

    void setupProblem(const std::vector<imu_sample_type>& samples);
    void solve(const std::vector<imu_sample_type>& samples);
};

/// Compute the mean of a vector with begin and end iterators.
template <class InputIt> double mean(InputIt begin, InputIt end) {
    const size_t n = end - begin;
    return std::accumulate(begin, end, 0.0) / (double)n;
}

/// Compute the variance of a vector with begin and end iterators.
template <class InputIt> double variance(InputIt begin, InputIt end) {
    const double sampleMean = mean(begin, end);
    double sum = 0;
    for (InputIt it = begin; it != end; it++) {
        const double y = *it - sampleMean;
        sum += (y * y);
    }
    const size_t n = end - begin;
    return sum / (double)(n - 1);
}

/// Compute the Allan Variance of a vector of samples.
class AllanVarianceEstimator {
public:
    AllanVarianceEstimator(const std::vector<double>& samples, const size_t maxChunkSize);
    ~AllanVarianceEstimator() = default;

    /// Get the white noise standard deviation
    double noiseDeviation() const { return m_noiseDeviation; }

    /// Get the allan deviation, i.e. variance of the chunk means. This value
    /// corresponds to the bias sigma.
    double allanDeviation() const { return m_allanDeviation; }

private:
    double m_noiseDeviation;
    double m_allanDeviation;

    void solve(const std::vector<double>& samples, const size_t maxChunkSize);
};
}
