
#include "packages/imu_propagator/include/imu_database.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace imu_propagator;

TEST(ImuSample, regression) {
    typedef details::ImuSample<double, double> imu_sample_type;

    const imu_sample_type::gyro_type gyro(1, 2, 3);
    const imu_sample_type::accel_type accel(4, 5, 6);
    const imu_sample_type sample(10, gyro, accel);

    EXPECT_EQ(10, sample.timestamp());
    EXPECT_EQ(1, sample.gyro()(0, 0));
    EXPECT_EQ(2, sample.gyro()(1, 0));
    EXPECT_EQ(3, sample.gyro()(2, 0));
    EXPECT_EQ(4, sample.accel()(0, 0));
    EXPECT_EQ(5, sample.accel()(1, 0));
    EXPECT_EQ(6, sample.accel()(2, 0));
}

TEST(ImuDatabase, adjacentSamplesAtTimeLeft) {
    typedef details::ImuSample<double, double> imu_sample_type;
    typedef ImuDatabase<double, double> imu_database_type;

    imu_database_type imuDatabase(10);

    const imu_sample_type::gyro_type gyro1(1, 2, 3);
    const imu_sample_type::accel_type accel1(4, 5, 6);
    const imu_sample_type sample1(10, gyro1, accel1);

    const imu_sample_type::gyro_type gyro2(7, 8, 9);
    const imu_sample_type::accel_type accel2(10, 11, 12);
    const imu_sample_type sample2(11, gyro2, accel2);

    imuDatabase.addImuSample(sample1);
    imuDatabase.addImuSample(sample2);

    const imu_database_type::imu_sample_pair_type pair = imuDatabase.adjacentSamplesAtTime(10);

    EXPECT_EQ(sample1.timestamp(), std::get<0>(pair).timestamp());
    EXPECT_EQ(sample2.timestamp(), std::get<1>(pair).timestamp());
}

TEST(ImuDatabase, adjacentSamplesAtTimeMidpoint) {
    typedef details::ImuSample<double, double> imu_sample_type;
    typedef ImuDatabase<double, double> imu_database_type;

    imu_database_type imuDatabase(10);

    const imu_sample_type::gyro_type gyro1(1, 2, 3);
    const imu_sample_type::accel_type accel1(4, 5, 6);
    const imu_sample_type sample1(10, gyro1, accel1);

    const imu_sample_type::gyro_type gyro2(7, 8, 9);
    const imu_sample_type::accel_type accel2(10, 11, 12);
    const imu_sample_type sample2(11, gyro2, accel2);

    imuDatabase.addImuSample(sample1);
    imuDatabase.addImuSample(sample2);

    const imu_database_type::imu_sample_pair_type pair = imuDatabase.adjacentSamplesAtTime(10.5);

    EXPECT_EQ(sample1.timestamp(), std::get<0>(pair).timestamp());
    EXPECT_EQ(sample2.timestamp(), std::get<1>(pair).timestamp());
}

TEST(ImuDatabase, adjacentSamplesAtTimeRight) {
    typedef details::ImuSample<double, double> imu_sample_type;
    typedef ImuDatabase<double, double> imu_database_type;

    imu_database_type imuDatabase(10);

    const imu_sample_type::gyro_type gyro1(1, 2, 3);
    const imu_sample_type::accel_type accel1(4, 5, 6);
    const imu_sample_type sample1(10, gyro1, accel1);

    const imu_sample_type::gyro_type gyro2(7, 8, 9);
    const imu_sample_type::accel_type accel2(10, 11, 12);
    const imu_sample_type sample2(11, gyro2, accel2);

    imuDatabase.addImuSample(sample1);
    imuDatabase.addImuSample(sample2);

    ASSERT_THROW(imuDatabase.adjacentSamplesAtTime(11), std::domain_error);
}

TEST(ImuDatabase, inRangeWithValuesInDatabase) {
    typedef details::ImuSample<double, double> imu_sample_type;
    typedef ImuDatabase<double, double> imu_database_type;

    imu_database_type imuDatabase(10);

    const imu_sample_type::gyro_type gyro1(1, 2, 3);
    const imu_sample_type::accel_type accel1(4, 5, 6);
    const imu_sample_type sample1(10, gyro1, accel1);

    const imu_sample_type::gyro_type gyro2(7, 8, 9);
    const imu_sample_type::accel_type accel2(10, 11, 12);
    const imu_sample_type sample2(11, gyro2, accel2);

    imuDatabase.addImuSample(sample1);
    imuDatabase.addImuSample(sample2);

    const imu_database_type::list_type data = imuDatabase.inRange(std::make_tuple(10., 11.));

    EXPECT_EQ(2, data.size());
    EXPECT_EQ(10, data.front().timestamp());
    EXPECT_EQ(11, data.back().timestamp());
}

TEST(ImuDatabase, inRangeRightNotInDatabase) {
    typedef details::ImuSample<double, double> imu_sample_type;
    typedef ImuDatabase<double, double> imu_database_type;

    imu_database_type imuDatabase(10);

    const imu_sample_type::gyro_type gyro1(1, 2, 3);
    const imu_sample_type::accel_type accel1(4, 5, 6);
    const imu_sample_type sample1(10, gyro1, accel1);

    const imu_sample_type::gyro_type gyro2(7, 8, 9);
    const imu_sample_type::accel_type accel2(10, 11, 12);
    const imu_sample_type sample2(11, gyro2, accel2);

    imuDatabase.addImuSample(sample1);
    imuDatabase.addImuSample(sample2);

    const imu_database_type::list_type data = imuDatabase.inRange(std::make_tuple(10., 10.5));

    EXPECT_EQ(1, data.size());
    EXPECT_EQ(10, data.front().timestamp());
}

TEST(ImuDatabase, inRangeLeftRightNotInDatabase) {
    typedef details::ImuSample<double, double> imu_sample_type;
    typedef ImuDatabase<double, double> imu_database_type;

    imu_database_type imuDatabase(10);

    const imu_sample_type::gyro_type gyro1(1, 2, 3);
    const imu_sample_type::accel_type accel1(4, 5, 6);
    const imu_sample_type sample1(10, gyro1, accel1);

    const imu_sample_type::gyro_type gyro2(7, 8, 9);
    const imu_sample_type::accel_type accel2(10, 11, 12);
    const imu_sample_type sample2(11, gyro2, accel2);

    imuDatabase.addImuSample(sample1);
    imuDatabase.addImuSample(sample2);

    const imu_database_type::list_type data = imuDatabase.inRange(std::make_tuple(10.4, 10.6));

    EXPECT_EQ(0, data.size());
}

TEST(LinearImuInterpolator, regression) {
    typedef details::ImuSample<double, double> imu_sample_type;
    typedef ImuDatabase<double, double> imu_database_type;
    typedef LinearImuInterpolator<double, double> linear_imu_interpolator_type;

    imu_database_type imuDatabase(10);

    const imu_sample_type::gyro_type gyro1(1, 2, 3);
    const imu_sample_type::accel_type accel1(4, 5, 6);
    const imu_sample_type sample1(10, gyro1, accel1);

    const imu_sample_type::gyro_type gyro2(7, 8, 9);
    const imu_sample_type::accel_type accel2(10, 11, 12);
    const imu_sample_type sample2(11, gyro2, accel2);

    const imu_sample_type::gyro_type gyro3(13, 14, 15);
    const imu_sample_type::accel_type accel3(16, 17, 18);
    const imu_sample_type sample3(12, gyro3, accel3);

    imuDatabase.addImuSample(sample1);
    imuDatabase.addImuSample(sample2);
    imuDatabase.addImuSample(sample3);

    linear_imu_interpolator_type interpolator(imuDatabase);
    const imu_sample_type imuSample = interpolator.getImuSampleAtTime(10.5);
    EXPECT_EQ(10.5, imuSample.timestamp());
    EXPECT_EQ(4, imuSample.gyro()(0, 0));
    EXPECT_EQ(5, imuSample.gyro()(1, 0));
    EXPECT_EQ(6, imuSample.gyro()(2, 0));
    EXPECT_EQ(7, imuSample.accel()(0, 0));
    EXPECT_EQ(8, imuSample.accel()(1, 0));
    EXPECT_EQ(9, imuSample.accel()(2, 0));
}
