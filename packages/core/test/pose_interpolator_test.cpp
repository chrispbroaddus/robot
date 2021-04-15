#include "../include/pose_interpolator.h"

#include "gtest/gtest.h"

#include <limits>
#include <map>
#include <memory>

namespace {
template <typename T> class PoseInterpolatorTest : public ::testing::Test {};

using TypesToTest = ::testing::Types<double, float>;

TYPED_TEST_CASE(PoseInterpolatorTest, TypesToTest);

TYPED_TEST(PoseInterpolatorTest, constructThrowsOnNonFiniteTimes) {
    using scalar_type = TypeParam;
    using map_type = std::map<scalar_type, Sophus::SE3<scalar_type> >;
    using interpolator_type = core::PoseInterpolator<scalar_type>;

    constexpr scalar_type invalid_times[] = { -std::numeric_limits<scalar_type>::infinity(), std::numeric_limits<scalar_type>::infinity(),
        std::numeric_limits<scalar_type>::quiet_NaN(), std::numeric_limits<scalar_type>::signaling_NaN() };

    for (auto value : invalid_times) {
        map_type data;
        // side effects are awesome (referencing this item in this way causes a new entry into the map where the value is
        // default-constructed)
        data[value];
        EXPECT_THROW(std::make_shared<interpolator_type>(data), std::invalid_argument);
    }
}

TYPED_TEST(PoseInterpolatorTest, interpolateThrowsOnEmptyContainer) {
    using scalar_type = TypeParam;
    using map_type = std::map<scalar_type, Sophus::SE3<scalar_type> >;
    using interpolator_type = core::PoseInterpolator<scalar_type>;

    map_type poses;
    interpolator_type interpolator(poses);
    EXPECT_THROW(interpolator.interpolateAtTime(0), std::invalid_argument);
}

TYPED_TEST(PoseInterpolatorTest, interpolateThrowsOnSingtletonContainer) {
    using scalar_type = TypeParam;
    using map_type = std::map<scalar_type, Sophus::SE3<scalar_type> >;
    using interpolator_type = core::PoseInterpolator<scalar_type>;

    map_type poses;
    poses[0];
    interpolator_type interpolator(poses);
    EXPECT_THROW(interpolator.interpolateAtTime(0), std::invalid_argument);
}

TYPED_TEST(PoseInterpolatorTest, interpolateThrowsOnNonfiniteTime) {
    using scalar_type = TypeParam;
    using map_type = std::map<scalar_type, Sophus::SE3<scalar_type> >;
    using interpolator_type = core::PoseInterpolator<scalar_type>;

    map_type poses;
    poses[0];
    poses[1].translation()(0) = 10;
    interpolator_type interpolator(poses);

    constexpr scalar_type invalid_times[] = { -std::numeric_limits<scalar_type>::infinity(), std::numeric_limits<scalar_type>::infinity(),
        std::numeric_limits<scalar_type>::quiet_NaN(), std::numeric_limits<scalar_type>::signaling_NaN() };

    for (auto value : invalid_times) {
        EXPECT_THROW(interpolator.interpolateAtTime(value), std::invalid_argument);
    }
}

TYPED_TEST(PoseInterpolatorTest, interpolateThrowsOnTimeBeforeMinimum) {
    using scalar_type = TypeParam;
    using map_type = std::map<scalar_type, Sophus::SE3<scalar_type> >;
    using interpolator_type = core::PoseInterpolator<scalar_type>;

    map_type poses;
    poses[0];
    poses[1].translation()(0) = 10;
    interpolator_type interpolator(poses);
    EXPECT_THROW(interpolator.interpolateAtTime(-1), std::invalid_argument);
}

TYPED_TEST(PoseInterpolatorTest, interpolateThrowsOnMaximum) {
    using scalar_type = TypeParam;
    using map_type = std::map<scalar_type, Sophus::SE3<scalar_type> >;
    using interpolator_type = core::PoseInterpolator<scalar_type>;

    map_type poses;
    poses[0];
    poses[1].translation()(0) = 10;
    interpolator_type interpolator(poses);
    EXPECT_THROW(interpolator.interpolateAtTime(poses.rbegin()->first), std::invalid_argument);
}

TYPED_TEST(PoseInterpolatorTest, interpolateExactlyOnIntervalStart) {
    using scalar_type = TypeParam;
    using map_type = std::map<scalar_type, Sophus::SE3<scalar_type> >;
    using interpolator_type = core::PoseInterpolator<scalar_type>;

    map_type poses;
    poses[0];
    poses[1].translation()(0) = 10;
    interpolator_type interpolator(poses);
    const auto interpolated = interpolator.interpolateAtTime(poses.begin()->first);
    const auto magnitudeLog = interpolated.log().norm();
    EXPECT_EQ(0, magnitudeLog);
}

TYPED_TEST(PoseInterpolatorTest, interpolateInsideInterval) {
    using scalar_type = TypeParam;
    using map_type = std::map<scalar_type, Sophus::SE3<scalar_type> >;
    using interpolator_type = core::PoseInterpolator<scalar_type>;

    map_type poses;
    poses[0];
    poses[1].translation()(0) = 10;
    interpolator_type interpolator(poses);
    const auto interpolated = interpolator.interpolateAtTime((poses.begin()->first + poses.rbegin()->first) / 2);
    EXPECT_NEAR(poses[1].translation()(0) / 2, interpolated.translation()(0), std::numeric_limits<TypeParam>::epsilon());
}
}