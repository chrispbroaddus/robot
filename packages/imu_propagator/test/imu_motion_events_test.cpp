
#include "packages/imu_propagator/include/imu_motion_events.h"
#include "gtest/gtest.h"

using namespace imu_propagator;

TEST(ImuMotionEvents, largeAccelerometerValue) {
    using motion_events = ImuMotionEvents<double, double>;
    using imu_sample = motion_events::imu_sample;

    constexpr double gyroMeanThreshold = 5 * M_PI / 180;
    constexpr double gyroStandardDeviationThreshold = 3 * M_PI / 180;
    constexpr double accelMeanThreshold = 10;
    constexpr double accelStandardDeviationThreshold = 1;
    constexpr double gravityInMetersPerSecond = 9.8;

    motion_events motionEvents(10, gyroMeanThreshold, gyroStandardDeviationThreshold, accelMeanThreshold, accelStandardDeviationThreshold,
        gravityInMetersPerSecond);

    motionEvents.update(imu_sample(1, imu_sample::gyro_type(0, 0, 0), imu_sample::accel_type(0, 0, gravityInMetersPerSecond)));
    EXPECT_TRUE(motionEvents.isStationary());

    motionEvents.update(imu_sample(1, imu_sample::gyro_type(0, 0, 0), imu_sample::accel_type(1000, 0, gravityInMetersPerSecond)));
    EXPECT_FALSE(motionEvents.isStationary());
}

TEST(ImuMotionEvents, largeGyroscopeValue) {
    using motion_events = ImuMotionEvents<double, double>;
    using imu_sample = motion_events::imu_sample;

    constexpr double gyroMeanThreshold = 5 * M_PI / 180;
    constexpr double gyroStandardDeviationThreshold = 3 * M_PI / 180;
    constexpr double accelMeanThreshold = 10;
    constexpr double accelStandardDeviationThreshold = 1;
    constexpr double gravityInMetersPerSecond = 9.8;

    motion_events motionEvents(10, gyroMeanThreshold, gyroStandardDeviationThreshold, accelMeanThreshold, accelStandardDeviationThreshold,
        gravityInMetersPerSecond);

    motionEvents.update(imu_sample(1, imu_sample::gyro_type(0, 0, 0), imu_sample::accel_type(0, 0, gravityInMetersPerSecond)));
    EXPECT_TRUE(motionEvents.isStationary());

    motionEvents.update(imu_sample(1, imu_sample::gyro_type(0, 100, 0), imu_sample::accel_type(0, 0, gravityInMetersPerSecond)));
    EXPECT_FALSE(motionEvents.isStationary());
}
