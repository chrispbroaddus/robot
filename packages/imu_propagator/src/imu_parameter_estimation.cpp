
#include "packages/imu_propagator/include/imu_parameter_estimation.h"
#include <stdexcept>

using namespace imu_propagator;

ImuParameterEstimation::ImuParameterEstimation(const std::vector<imu_sample_type>& samples, const size_t maxChunkSize)
    : m_maxChunkSize(maxChunkSize)
    , m_meanDt(0) {
    solve(samples);
}

void ImuParameterEstimation::setupProblem(const std::vector<imu_sample_type>& samples) {
    for (const auto& sample : samples) {
        m_time.push_back(sample.timestamp());

        m_gyroX.push_back(sample.gyro()(0));
        m_gyroY.push_back(sample.gyro()(1));
        m_gyroZ.push_back(sample.gyro()(2));

        m_accelX.push_back(sample.accel()(0));
        m_accelY.push_back(sample.accel()(1));
        m_accelZ.push_back(sample.accel()(2));
    }

    std::vector<double> deltaTimes;
    for (size_t i = 0; (i + 1) < m_time.size(); i++) {
        deltaTimes.push_back(m_time[i + 1] - m_time[i]);
    }
    m_meanDt = mean(deltaTimes.begin(), deltaTimes.end());
}

void ImuParameterEstimation::solve(const std::vector<imu_sample_type>& samples) {
    setupProblem(samples);

    const AllanVarianceEstimator gyroXAllanVarianceEstimator(m_gyroX, m_maxChunkSize);
    const AllanVarianceEstimator gyroYAllanVarianceEstimator(m_gyroY, m_maxChunkSize);
    const AllanVarianceEstimator gyroZAllanVarianceEstimator(m_gyroZ, m_maxChunkSize);

    const AllanVarianceEstimator accelXAllanVarianceEstimator(m_accelX, m_maxChunkSize);
    const AllanVarianceEstimator accelYAllanVarianceEstimator(m_accelY, m_maxChunkSize);
    const AllanVarianceEstimator accelZAllanVarianceEstimator(m_accelZ, m_maxChunkSize);

    m_gyroWhiteNoiseSigma(0) = gyroXAllanVarianceEstimator.noiseDeviation() * std::sqrt(m_meanDt);
    m_gyroWhiteNoiseSigma(1) = gyroYAllanVarianceEstimator.noiseDeviation() * std::sqrt(m_meanDt);
    m_gyroWhiteNoiseSigma(2) = gyroZAllanVarianceEstimator.noiseDeviation() * std::sqrt(m_meanDt);

    m_gyroBiasSigma(0) = gyroXAllanVarianceEstimator.allanDeviation() * (1.0 / std::sqrt(m_meanDt));
    m_gyroBiasSigma(1) = gyroYAllanVarianceEstimator.allanDeviation() * (1.0 / std::sqrt(m_meanDt));
    m_gyroBiasSigma(2) = gyroZAllanVarianceEstimator.allanDeviation() * (1.0 / std::sqrt(m_meanDt));

    m_accelWhiteNoiseSigma(0) = accelXAllanVarianceEstimator.noiseDeviation() * std::sqrt(m_meanDt);
    m_accelWhiteNoiseSigma(1) = accelYAllanVarianceEstimator.noiseDeviation() * std::sqrt(m_meanDt);
    m_accelWhiteNoiseSigma(2) = accelZAllanVarianceEstimator.noiseDeviation() * std::sqrt(m_meanDt);

    m_accelBiasSigma(0) = accelXAllanVarianceEstimator.allanDeviation() * (1.0 / std::sqrt(m_meanDt));
    m_accelBiasSigma(1) = accelYAllanVarianceEstimator.allanDeviation() * (1.0 / std::sqrt(m_meanDt));
    m_accelBiasSigma(2) = accelZAllanVarianceEstimator.allanDeviation() * (1.0 / std::sqrt(m_meanDt));
}

AllanVarianceEstimator::AllanVarianceEstimator(const std::vector<double>& samples, const size_t maxChunkSize)
    : m_noiseDeviation(0)
    , m_allanDeviation(0) {
    solve(samples, maxChunkSize);
}

void AllanVarianceEstimator::solve(const std::vector<double>& samples, const size_t maxChunkSize) {
    std::vector<double> varianceChunkMeam;
    std::vector<double> meanChunkVariance;
    std::vector<double> allanVariance;

    for (size_t chunkSize = 2; chunkSize <= maxChunkSize; chunkSize++) {
        std::vector<double> chunkMean, chunkVariance;
        for (size_t nextChunk = 0; nextChunk + chunkSize < samples.size(); nextChunk += chunkSize) {
            chunkMean.push_back(mean(samples.begin() + nextChunk, samples.begin() + nextChunk + chunkSize));
            chunkVariance.push_back(variance(samples.begin() + nextChunk, samples.begin() + nextChunk + chunkSize));
        }

        if (chunkMean.size() < 2) {
            throw std::runtime_error("not enough data");
        }

        varianceChunkMeam.push_back(variance(chunkMean.begin(), chunkMean.end()));
        meanChunkVariance.push_back(mean(chunkVariance.begin(), chunkVariance.end()));

        std::vector<double> residuals;
        for (size_t i = 0; (i + 1) < chunkMean.size(); i++) {
            residuals.push_back(chunkMean[i + 1] - chunkMean[i]);
        }
        allanVariance.push_back(variance(residuals.begin(), residuals.end()));
    }

    size_t minIndex = 0;
    double minValue = allanVariance[minIndex];
    for (size_t i = 1; i < allanVariance.size(); i++) {
        if (allanVariance[i] < minValue) {
            minValue = allanVariance[i];
            minIndex = i;
        }
    }

    m_noiseDeviation = std::sqrt(meanChunkVariance[minIndex]);
    m_allanDeviation = std::sqrt(allanVariance[minIndex]);
}
