#include "packages/imu_propagator/include/imu_parameter_estimation.h"
#include "gtest/gtest.h"
#include <random>

using namespace imu_propagator;

TEST(AllanVarianceEstimator, simulatedCheck) {
    constexpr size_t numSamples = 1e5;
    constexpr double whiteNoiseSigma = 1e-3;
    constexpr double biasSigma = 1e-5;

    std::vector<double> samples(numSamples);

    std::default_random_engine generator(1234);
    std::normal_distribution<double> biasDistribution(0, biasSigma);
    std::normal_distribution<double> whiteNoiseDistribution(0, whiteNoiseSigma);

    double bias = 0;
    for (size_t i = 0; i < numSamples; i++) {
        bias = bias + biasDistribution(generator);
        samples[i] = whiteNoiseDistribution(generator) + bias;
    }

    const AllanVarianceEstimator estimator(samples, 1000);

    const double whiteNoiseErrorScale = estimator.noiseDeviation() / whiteNoiseSigma;
    const double biaseErrorScale = estimator.allanDeviation() / biasSigma;

    EXPECT_NEAR(1, whiteNoiseErrorScale, 1e-3);
    EXPECT_NEAR(1, biaseErrorScale, 20);
}

TEST(ImuParameterEstimation, simulatedCheck) {
    constexpr size_t numSamples = 1e5;
    constexpr double gyroXWhiteNoiseSigma = 1e-3;
    constexpr double gyroYWhiteNoiseSigma = 1e-3;
    constexpr double gyroZWhiteNoiseSigma = 1e-3;
    constexpr double gyroXBiasSigma = 1e-5;
    constexpr double gyroYBiasSigma = 1e-5;
    constexpr double gyroZBiasSigma = 1e-5;
    constexpr double accelXWhiteNoiseSigma = 1e-3;
    constexpr double accelYWhiteNoiseSigma = 1e-3;
    constexpr double accelZWhiteNoiseSigma = 1e-3;
    constexpr double accelXBiasSigma = 1e-5;
    constexpr double accelYBiasSigma = 1e-5;
    constexpr double accelZBiasSigma = 1e-5;

    std::vector<ImuParameterEstimation::imu_sample_type> samples;

    std::default_random_engine generator(1234);

    std::normal_distribution<double> gyroXWhiteNoiseDistribution(0, gyroXWhiteNoiseSigma);
    std::normal_distribution<double> gyroYWhiteNoiseDistribution(0, gyroYWhiteNoiseSigma);
    std::normal_distribution<double> gyroZWhiteNoiseDistribution(0, gyroZWhiteNoiseSigma);
    std::normal_distribution<double> gyroXBiasDistribution(0, gyroXBiasSigma);
    std::normal_distribution<double> gyroYBiasDistribution(0, gyroYBiasSigma);
    std::normal_distribution<double> gyroZBiasDistribution(0, gyroZBiasSigma);

    std::normal_distribution<double> accelXWhiteNoiseDistribution(0, accelXWhiteNoiseSigma);
    std::normal_distribution<double> accelYWhiteNoiseDistribution(0, accelYWhiteNoiseSigma);
    std::normal_distribution<double> accelZWhiteNoiseDistribution(0, accelZWhiteNoiseSigma);
    std::normal_distribution<double> accelXBiasDistribution(0, accelXBiasSigma);
    std::normal_distribution<double> accelYBiasDistribution(0, accelYBiasSigma);
    std::normal_distribution<double> accelZBiasDistribution(0, accelZBiasSigma);

    ImuParameterEstimation::imu_sample_type::gyro_type gyroBias(0, 0, 0);
    ImuParameterEstimation::imu_sample_type::accel_type accelBias(0, 0, 0);
    for (size_t i = 0; i < numSamples; i++) {
        gyroBias(0) += gyroXBiasDistribution(generator);
        gyroBias(1) += gyroYBiasDistribution(generator);
        gyroBias(2) += gyroZBiasDistribution(generator);

        accelBias(0) += accelXBiasDistribution(generator);
        accelBias(1) += accelYBiasDistribution(generator);
        accelBias(2) += accelZBiasDistribution(generator);

        ImuParameterEstimation::imu_sample_type::gyro_type gyro;
        gyro(0) = gyroXWhiteNoiseDistribution(generator) + gyroBias(0);
        gyro(1) = gyroYWhiteNoiseDistribution(generator) + gyroBias(1);
        gyro(2) = gyroZWhiteNoiseDistribution(generator) + gyroBias(2);

        ImuParameterEstimation::imu_sample_type::accel_type accel;
        accel(0) = accelXWhiteNoiseDistribution(generator) + accelBias(0);
        accel(1) = accelYWhiteNoiseDistribution(generator) + accelBias(1);
        accel(2) = accelZWhiteNoiseDistribution(generator) + accelBias(2);

        samples.push_back(ImuParameterEstimation::imu_sample_type((double)i, gyro, accel));
    }

    const ImuParameterEstimation estimator(samples, ImuParameterEstimation::kDefaultMaxChunkSize);

    EXPECT_NEAR(1, estimator.gyroWhiteNoiseSigma()(0) / gyroXWhiteNoiseSigma, 1e-2);
    EXPECT_NEAR(1, estimator.gyroWhiteNoiseSigma()(1) / gyroYWhiteNoiseSigma, 1e-2);
    EXPECT_NEAR(1, estimator.gyroWhiteNoiseSigma()(2) / gyroZWhiteNoiseSigma, 1e-2);
    EXPECT_NEAR(1, estimator.accelWhiteNoiseSigma()(0) / accelXWhiteNoiseSigma, 1e-2);
    EXPECT_NEAR(1, estimator.accelWhiteNoiseSigma()(1) / accelYWhiteNoiseSigma, 1e-2);
    EXPECT_NEAR(1, estimator.accelWhiteNoiseSigma()(2) / accelZWhiteNoiseSigma, 1e-2);

    EXPECT_NEAR(1, estimator.gyroBiasSigma()(0) / gyroXBiasSigma, 20);
    EXPECT_NEAR(1, estimator.gyroBiasSigma()(1) / gyroYBiasSigma, 20);
    EXPECT_NEAR(1, estimator.gyroBiasSigma()(2) / gyroZBiasSigma, 20);
    EXPECT_NEAR(1, estimator.accelBiasSigma()(0) / accelXBiasSigma, 20);
    EXPECT_NEAR(1, estimator.accelBiasSigma()(1) / accelYBiasSigma, 20);
    EXPECT_NEAR(1, estimator.accelBiasSigma()(2) / accelZBiasSigma, 20);
}
