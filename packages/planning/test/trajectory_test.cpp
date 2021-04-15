#include "glog/logging.h"
#include "wykobi/wykobi.hpp"
#include "gtest/gtest.h"

#include "packages/perception/grid.h"
#include "packages/planning/planner.h"
#include "packages/planning/proto/path.pb.h"
#include "packages/planning/proto/trajectory_options.pb.h"
#include "packages/planning/proto/trajectory_planner_options.pb.h"
#include "packages/planning/utils.h"

#include "packages/planning/test/common.h"

namespace planning {

TEST(planner, squareTrajectory) {
    const float arc_length = 1.0;
    const float curvature = 1.0;
    TrajectoryOptions options = loadDefaultTrajectoryOptions();
    SquareTrajectory generator(options);
    Trajectory trajectory;
    ASSERT_TRUE(generator.generate(arc_length, curvature, trajectory));
    ASSERT_TRUE(trajectory.type() == TrajectoryType::SQUARE);
    ASSERT_TRUE(trajectory.elements_size() == planning::test::kExpectedNumberSegments);
}

TEST(planner, squareTrajectoryInterpolation) {
    const float arc_length = 1.0;
    const float curvature = 1.0;
    TrajectoryOptions options = loadDefaultTrajectoryOptions();
    SquareTrajectory generator(options);
    Trajectory trajectory;
    ASSERT_TRUE(generator.generate(arc_length, curvature, trajectory));
    ASSERT_TRUE(trajectory.type() == TrajectoryType::SQUARE);
    ASSERT_TRUE(trajectory.elements_size() == planning::test::kExpectedNumberSegments);
    LOG(INFO) << trajectory.DebugString();
    test::TrajectoryVelocityInterpolator<0> zeroOrderInterpolator;
    zeroOrderInterpolator.interpolate(trajectory);
}

TEST(planner, squareTrajectoryWithDeceleration) {
    TrajectoryOptions options = loadDefaultTrajectoryOptions();
    options.set_max_deceleration(0.5);

    SquareTrajectoryWithDeceleration generator(options);
    Trajectory trajectory;
    const float arc_length = 1.0;
    const float curvature = 1.0;
    ASSERT_TRUE(generator.generate(arc_length, curvature, trajectory));
    ASSERT_TRUE(trajectory.type() == TrajectoryType::SQUARE);

    for (const auto& element : *trajectory.mutable_elements()) {
        ASSERT_LE(element.linear_velocity(), options.max_velocity());
        ASSERT_GE(element.linear_velocity(), 0.0);
    }
}

TEST(planner, triangularTrapezoidalTrajectory) {
    const double arc_length = 1.0;
    const double curvature = 1.0;
    TrajectoryOptions options;
    options.set_frequency(10);
    options.set_max_velocity(1.0);
    options.set_max_acceleration(1.0);
    options.set_max_deceleration(options.max_acceleration());

    constexpr int kDefaultFrequency = 10;
    constexpr int64_t kNanosecondsPerSecond = 1000000000;
    Trajectory trajectory;
    TrapezoidalTrajectory generator(options);
    ASSERT_TRUE(generator.generate(arc_length, curvature, trajectory));
    ASSERT_TRUE(trajectory.type() == TrajectoryType::TRIANGULAR);
    ASSERT_TRUE(trajectory.elements_size() > 1);
    for (int i = 1; i < trajectory.elements_size(); ++i) {
        ASSERT_GT(trajectory.elements(i).relative_time().nanos(), trajectory.elements(i - 1).relative_time().nanos());
        const int64_t startNanos = trajectory.elements(i).relative_time().nanos();
        const int64_t endNanos = trajectory.elements(i - 1).relative_time().nanos();
        const int64_t diff = startNanos - endNanos;

        ASSERT_GT(diff, 0);
        ASSERT_LT(std::abs(diff - kNanosecondsPerSecond / kDefaultFrequency), 100);
        ASSERT_GE(trajectory.elements(i).linear_velocity(), 0.0);
        ASSERT_LE(trajectory.elements(i).linear_velocity(), options.max_velocity());
    }
    ASSERT_FLOAT_EQ(trajectory.elements(trajectory.elements_size() - 1).linear_velocity(), 0.0);
}

TEST(planner, trapezoidalTrajectory) {
    const float arc_length = 10.0;
    const float curvature = 1.0;

    TrajectoryOptions options;
    options.set_frequency(10);
    options.set_max_velocity(1.0);
    options.set_max_acceleration(5.0);
    options.set_max_deceleration(options.max_acceleration());

    TrapezoidalTrajectory generator(options);
    Trajectory trajectory;
    ASSERT_TRUE(generator.generate(arc_length, curvature, trajectory));
    ASSERT_TRUE(trajectory.type() == TrajectoryType::TRAPEZOIDAL);
    ASSERT_TRUE(trajectory.elements_size() > 1);
    for (int i = 1; i < trajectory.elements_size(); ++i) {
        ASSERT_GT(trajectory.elements(i).relative_time().nanos(), trajectory.elements(i - 1).relative_time().nanos());
        auto diff = trajectory.elements(i).relative_time().nanos() - trajectory.elements(i - 1).relative_time().nanos();
        ASSERT_GT(diff, 0.0);
        ASSERT_GE(trajectory.elements(i).linear_velocity(), 0.0);
        ASSERT_LE(trajectory.elements(i).linear_velocity(), options.max_velocity());
    }
    ASSERT_FLOAT_EQ(trajectory.elements(trajectory.elements_size() - 1).linear_velocity(), 0.0);
}
} // planning

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
