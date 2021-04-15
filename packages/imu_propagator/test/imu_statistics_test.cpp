
#include "packages/imu_propagator/include/imu_statistics.h"
#include "gtest/gtest.h"

using namespace imu_propagator;

TEST(MovingStatistics, regression) {
    MovingStatistics<double> movingStatistics(4);

    movingStatistics.push(1);
    EXPECT_DOUBLE_EQ(1, movingStatistics.mean());
    EXPECT_DOUBLE_EQ(0, movingStatistics.variance());

    movingStatistics.push(2);
    EXPECT_DOUBLE_EQ(1.5, movingStatistics.mean());
    EXPECT_DOUBLE_EQ(0.5, movingStatistics.variance());

    movingStatistics.push(3);
    EXPECT_DOUBLE_EQ(2, movingStatistics.mean());
    EXPECT_DOUBLE_EQ(1, movingStatistics.variance());

    movingStatistics.push(4);
    EXPECT_DOUBLE_EQ(2.5, movingStatistics.mean());
    EXPECT_DOUBLE_EQ(5.0 / 3.0, movingStatistics.variance());

    movingStatistics.push(5);
    EXPECT_DOUBLE_EQ(3.5, movingStatistics.mean());
    EXPECT_DOUBLE_EQ(5.0 / 3.0, movingStatistics.variance());

    movingStatistics.push(7);
    EXPECT_DOUBLE_EQ(4.75, movingStatistics.mean());
    EXPECT_NEAR(2.916666667, movingStatistics.variance(), 1e-7);
}

TEST(ImuStatistics, regression) {
    using imu_statistics = ImuStatistics<double, double>;
    using imu_sample = imu_statistics::imu_sample;

    imu_statistics imuStatistics(3);

    imuStatistics.update(imu_sample(0, imu_sample::gyro_type(0, 0, 0), imu_sample::accel_type(0, 0, 0)));
    imuStatistics.update(imu_sample(1, imu_sample::gyro_type(1, 2, 5), imu_sample::accel_type(5, 2, 1)));
    imuStatistics.update(imu_sample(2, imu_sample::gyro_type(2, 3, 6), imu_sample::accel_type(6, 3, 2)));
    imuStatistics.update(imu_sample(3, imu_sample::gyro_type(3, 4, 7), imu_sample::accel_type(7, 4, 3)));

    EXPECT_DOUBLE_EQ(2, imuStatistics.gyroX().mean());
    EXPECT_DOUBLE_EQ(3, imuStatistics.gyroY().mean());
    EXPECT_DOUBLE_EQ(6, imuStatistics.gyroZ().mean());

    EXPECT_DOUBLE_EQ(6, imuStatistics.accelX().mean());
    EXPECT_DOUBLE_EQ(3, imuStatistics.accelY().mean());
    EXPECT_DOUBLE_EQ(2, imuStatistics.accelZ().mean());
}