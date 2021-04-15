
#include "packages/imu_propagator/include/imu_orientation_estimator.h"
#include "gtest/gtest.h"

using namespace imu_propagator;

constexpr double gravity = 9.8;

TEST(ImuOrientationEstimator, gyroRotationX) {
    using imu_sample = ImuOrientationEstimator<double>::imu_sample;

    const double smallDegreeInRadians = 0.1 * M_PI / 180.0;

    ImuOrientationEstimator<double> filter(0, Eigen::Quaternion<double>(1, 0, 0, 0), 1);
    filter.update(imu_sample(1, imu_sample::gyro_type(smallDegreeInRadians, 0, 0), imu_sample::accel_type(0, 0, 0)));

    const Eigen::Vector3d rotatedUnitY = filter.quaternion().toRotationMatrix() * Eigen::Vector3d::UnitY();
    EXPECT_DOUBLE_EQ(0, rotatedUnitY(0));
    EXPECT_NEAR(std::cos(smallDegreeInRadians), rotatedUnitY(1), 1e-2);
    EXPECT_NEAR(std::sin(smallDegreeInRadians), rotatedUnitY(2), 1e-2);
}

TEST(ImuOrientationEstimator, gyroRotationY) {
    using imu_sample = ImuOrientationEstimator<double>::imu_sample;

    const double smallDegreeInRadians = 0.1 * M_PI / 180.0;

    ImuOrientationEstimator<double> filter(0, Eigen::Quaternion<double>(1, 0, 0, 0), 1);
    filter.update(imu_sample(1, imu_sample::gyro_type(smallDegreeInRadians, 0, 0), imu_sample::accel_type(0, 0, 0)));

    const Eigen::Vector3d rotatedUnitY = filter.quaternion().toRotationMatrix() * Eigen::Vector3d::UnitY();
    EXPECT_DOUBLE_EQ(0, rotatedUnitY(0));
    EXPECT_NEAR(std::cos(smallDegreeInRadians), rotatedUnitY(1), 1e-2);
    EXPECT_NEAR(-std::sin(smallDegreeInRadians), rotatedUnitY(2), 1e-2);
}

TEST(ImuOrientationEstimator, gyroRotationZ) {
    using imu_sample = ImuOrientationEstimator<double>::imu_sample;

    const double smallDegreeInRadians = 0.1 * M_PI / 180.0;

    ImuOrientationEstimator<double> filter(0, Eigen::Quaternion<double>(1, 0, 0, 0), 1);
    filter.update(imu_sample(1, imu_sample::gyro_type(0, 0, smallDegreeInRadians), imu_sample::accel_type(0, 0, 0)));

    const Eigen::Vector3d rotatedUnitX = filter.quaternion().toRotationMatrix() * Eigen::Vector3d::UnitX();
    EXPECT_NEAR(std::cos(smallDegreeInRadians), rotatedUnitX(0), 1e-5);
    EXPECT_NEAR(std::sin(smallDegreeInRadians), rotatedUnitX(1), 1e-2);
    EXPECT_DOUBLE_EQ(0, rotatedUnitX(2));
}

TEST(ImuOrientationEstimator, xUp) {
    using imu_sample = ImuOrientationEstimator<double>::imu_sample;

    constexpr double gravity = 9.8;
    ImuOrientationEstimator<double> filter(0, Eigen::Quaternion<double>(1, 0, 0, 0), 0);
    filter.update(imu_sample(1, imu_sample::gyro_type(0, 0, 0), imu_sample::accel_type(gravity, 0, 0)));

    const Eigen::Vector3d rotatedUnitX = filter.quaternion().toRotationMatrix() * Eigen::Vector3d::UnitX();
    EXPECT_NEAR(0, rotatedUnitX(0), 1e-12);
    EXPECT_NEAR(0, rotatedUnitX(1), 1e-12);
    EXPECT_NEAR(1, rotatedUnitX(2), 1e-12);

    const Eigen::Vector3d rotatedUnitY = filter.quaternion().toRotationMatrix() * Eigen::Vector3d::UnitY();
    EXPECT_NEAR(0, rotatedUnitY(0), 1e-12);
    EXPECT_NEAR(1, rotatedUnitY(1), 1e-12);
    EXPECT_NEAR(0, rotatedUnitY(2), 1e-12);

    const Eigen::Vector3d rotatedUnitZ = filter.quaternion().toRotationMatrix() * Eigen::Vector3d::UnitZ();
    EXPECT_NEAR(-1, rotatedUnitZ(0), 1e-12);
    EXPECT_NEAR(0, rotatedUnitZ(1), 1e-12);
    EXPECT_NEAR(0, rotatedUnitZ(2), 1e-12);
}

TEST(ImuOrientationEstimator, yUp) {
    using imu_sample = ImuOrientationEstimator<double>::imu_sample;

    constexpr double gravity = 9.8;
    ImuOrientationEstimator<double> filter(0, Eigen::Quaternion<double>(1, 0, 0, 0), 0);
    filter.update(imu_sample(1, imu_sample::gyro_type(0, 0, 0), imu_sample::accel_type(0, gravity, 0)));

    const Eigen::Vector3d rotatedUnitX = filter.quaternion().toRotationMatrix() * Eigen::Vector3d::UnitX();
    EXPECT_NEAR(1, rotatedUnitX(0), 1e-12);
    EXPECT_NEAR(0, rotatedUnitX(1), 1e-12);
    EXPECT_NEAR(0, rotatedUnitX(2), 1e-12);

    const Eigen::Vector3d rotatedUnitY = filter.quaternion().toRotationMatrix() * Eigen::Vector3d::UnitY();
    EXPECT_NEAR(0, rotatedUnitY(0), 1e-12);
    EXPECT_NEAR(0, rotatedUnitY(1), 1e-12);
    EXPECT_NEAR(1, rotatedUnitY(2), 1e-12);

    const Eigen::Vector3d rotatedUnitZ = filter.quaternion().toRotationMatrix() * Eigen::Vector3d::UnitZ();
    EXPECT_NEAR(0, rotatedUnitZ(0), 1e-12);
    EXPECT_NEAR(-1, rotatedUnitZ(1), 1e-12);
    EXPECT_NEAR(0, rotatedUnitZ(2), 1e-12);
}

TEST(ImuOrientationEstimator, zUp) {
    using imu_sample = ImuOrientationEstimator<double>::imu_sample;

    constexpr double gravity = 9.8;
    ImuOrientationEstimator<double> filter(0, Eigen::Quaternion<double>(1, 0, 0, 0), 0);
    filter.update(imu_sample(1, imu_sample::gyro_type(0, 0, 0), imu_sample::accel_type(0, 0, gravity)));

    const Eigen::Vector3d rotatedUnitX = filter.quaternion().toRotationMatrix() * Eigen::Vector3d::UnitX();
    EXPECT_DOUBLE_EQ(1, rotatedUnitX(0));
    EXPECT_DOUBLE_EQ(0, rotatedUnitX(1));
    EXPECT_DOUBLE_EQ(0, rotatedUnitX(2));

    const Eigen::Vector3d rotatedUnitY = filter.quaternion().toRotationMatrix() * Eigen::Vector3d::UnitY();
    EXPECT_DOUBLE_EQ(0, rotatedUnitY(0));
    EXPECT_DOUBLE_EQ(1, rotatedUnitY(1));
    EXPECT_DOUBLE_EQ(0, rotatedUnitY(2));

    const Eigen::Vector3d rotatedUnitZ = filter.quaternion().toRotationMatrix() * Eigen::Vector3d::UnitZ();
    EXPECT_DOUBLE_EQ(0, rotatedUnitZ(0));
    EXPECT_DOUBLE_EQ(0, rotatedUnitZ(1));
    EXPECT_DOUBLE_EQ(1, rotatedUnitZ(2));
}
