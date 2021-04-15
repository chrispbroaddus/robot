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

bool computeIntersectionPoint(const wykobi::point3d<double>& target, wykobi::point2d<double>& point);
double computeArcLength(const wykobi::point3d<double>& target, const wykobi::point2d<double>& intersection);

TEST(planner, configuration) {
    TrajectoryOptions trajectoryOptions = loadDefaultTrajectoryOptions();
    TrajectoryPlannerOptions trajectoryPlannerOptions = loadDefaultTrajectoryPlannerOptions();
    ArcPlannerOptions arcPlannerOptions = loadDefaultArcPlannerOptions();
}

TEST(planner, instantiate) {
    std::unique_ptr<ArcPlanner> planner_ptr;
    std::unique_ptr<TrajectoryGenerator> generator_ptr;
    ArcPlannerOptions options;
    planner_ptr.reset(new ArcPlanner(options, std::move(generator_ptr)));
    ASSERT_NE(planner_ptr.get(), nullptr);
}

TEST(planner, planTo) {
    ArcPlannerOptions options = loadDefaultArcPlannerOptions();
    TrajectoryOptions trajectory_options = loadDefaultTrajectoryOptions();

    std::unique_ptr<ArcPlanner> planner_ptr;
    std::unique_ptr<TrajectoryGenerator> generator_ptr(new SquareTrajectory(trajectory_options));

    planner_ptr.reset(new ArcPlanner(options, std::move(generator_ptr)));
    ASSERT_NE(planner_ptr.get(), nullptr);

    double x_pos = 10.0;
    double y_pos = -10.0;
    constexpr float kDefaultCurvatureTolerance = 0.001;

    Sophus::SE3d target;
    elementsToPose(x_pos, y_pos, 0.0, 0.0, 0.0, 0.0, target);

    {
        Trajectory trajectory;
        ASSERT_TRUE(planner_ptr->planTo(target, trajectory));
        auto element = trajectory.elements(0);
        ASSERT_FLOAT_EQ(element.curvature(), 1.0 / y_pos);
    }
    {
        elementsToPose(10.0, -5.0, 0.0, 0.0, 0.0, 0.0, target);
        Trajectory trajectory;
        ASSERT_TRUE(planner_ptr->planTo(target, trajectory));
        auto element = trajectory.elements(0);
        ASSERT_NEAR(element.curvature(), -1.0 / 12.5, kDefaultCurvatureTolerance);
    }
    {
        elementsToPose(10.0, 5.0, 0.0, 0.0, 0.0, 0.0, target);
        Trajectory trajectory;
        ASSERT_TRUE(planner_ptr->planTo(target, trajectory));
        auto element = trajectory.elements(0);
        ASSERT_NEAR(element.curvature(), 1.0 / 12.5, kDefaultCurvatureTolerance);
    }
}

TEST(planner, planToDirection) {

    ArcPlannerOptions options = loadDefaultArcPlannerOptions();
    TrajectoryOptions trajectory_options = loadDefaultTrajectoryOptions();

    std::unique_ptr<ArcPlanner> planner_ptr;
    std::unique_ptr<TrajectoryGenerator> generator_ptr(new SquareTrajectory(trajectory_options));
    planner_ptr.reset(new ArcPlanner(options, std::move(generator_ptr)));
    ASSERT_NE(planner_ptr.get(), nullptr);

    Sophus::SE3d target;

    {
        // Front,left
        int x_pos = 10;
        int y_pos = -10;
        target.translation()[kXAxis] = x_pos;
        target.translation()[kYAxis] = y_pos;
        Trajectory trajectory;
        ASSERT_TRUE(planner_ptr->planTo(target, trajectory));
        auto element = trajectory.elements(0);
        ASSERT_TRUE(element.linear_velocity() > 0);
    }
    {
        // Front, right
        int x_pos = 10;
        int y_pos = -10;
        target.translation()[kXAxis] = x_pos;
        target.translation()[kYAxis] = y_pos;
        Trajectory trajectory;
        ASSERT_TRUE(planner_ptr->planTo(target, trajectory));
        auto element = trajectory.elements(0);
        ASSERT_TRUE(element.linear_velocity() > 0);
    }
    {
        // Rear, right
        int x_pos = -10;
        int y_pos = -10;
        target.translation()[kXAxis] = x_pos;
        target.translation()[kYAxis] = y_pos;
        Trajectory trajectory;
        ASSERT_TRUE(planner_ptr->planTo(target, trajectory));
        auto element = trajectory.elements(0);
        ASSERT_TRUE(element.linear_velocity() < 0);
    }
    {
        // Rear, left
        int x_pos = -10;
        int y_pos = 10;
        target.translation()[kXAxis] = x_pos;
        target.translation()[kYAxis] = y_pos;
        Trajectory trajectory;
        ASSERT_TRUE(planner_ptr->planTo(target, trajectory));
        auto element = trajectory.elements(0);
        ASSERT_TRUE(element.linear_velocity() < 0);
    }
}

TEST(planner, planToDegenerateCurves) {

    ArcPlannerOptions options = loadDefaultArcPlannerOptions();
    TrajectoryOptions trajectory_options = loadDefaultTrajectoryOptions();

    std::unique_ptr<ArcPlanner> planner_ptr;
    std::unique_ptr<TrajectoryGenerator> generator_ptr(new SquareTrajectory(trajectory_options));
    planner_ptr.reset(new ArcPlanner(options, std::move(generator_ptr)));
    ASSERT_NE(planner_ptr.get(), nullptr);

    Sophus::SE3d target;

    {
        // Front
        int x_pos = 10;
        int y_pos = 0;
        target.translation()[kXAxis] = x_pos;
        target.translation()[kYAxis] = y_pos;
        Trajectory trajectory;
        ASSERT_TRUE(planner_ptr->planTo(target, trajectory));
        auto element = trajectory.elements(0);
        ASSERT_TRUE(element.linear_velocity() > 0);
    }
    {
        // Rear
        int x_pos = -10;
        int y_pos = 0;
        target.translation()[kXAxis] = x_pos;
        target.translation()[kYAxis] = y_pos;
        Trajectory trajectory;
        ASSERT_TRUE(planner_ptr->planTo(target, trajectory));
        auto element = trajectory.elements(0);
        ASSERT_TRUE(element.linear_velocity() < 0);
    }
}

TEST(planner, invalidPlanTo) {

    ArcPlannerOptions options = loadDefaultArcPlannerOptions();
    TrajectoryOptions trajectory_options = loadDefaultTrajectoryOptions();

    std::unique_ptr<ArcPlanner> planner_ptr;
    std::unique_ptr<TrajectoryGenerator> generator_ptr(new SquareTrajectory(trajectory_options));
    planner_ptr.reset(new ArcPlanner(options, std::move(generator_ptr)));
    ASSERT_NE(planner_ptr.get(), nullptr);

    constexpr double x_pos = std::numeric_limits<double>::infinity();
    Sophus::SE3d target;
    elementsToPose(x_pos, 0.0, 0.0, 0.0, 0.0, 0.0, target);
    Trajectory trajectory;
    ASSERT_FALSE(planner_ptr->planTo(target, trajectory));
}

TEST(planner, curveLength) {
    wykobi::point3d<double> target;
    target.x = 10;
    target.y = 10;
    target.z = 0;
    wykobi::point2d<double> intersection;
    intersection.x = 10;
    intersection.y = 0;
    const double arc_length = 10 * M_PI / 2.0;
    ASSERT_FLOAT_EQ(arc_length, computeArcLength(target, intersection));
}

TEST(planner, squareTrajectory) {
    const float arc_length = 1.0;
    const float curvature = 1.0;
    TrajectoryOptions options = loadDefaultTrajectoryOptions();
    SquareTrajectory generator(options);
    Trajectory trajectory;
    ASSERT_TRUE(generator.generate(arc_length, curvature, trajectory));
    ASSERT_TRUE(trajectory.type() == TrajectoryType::SQUARE);
    ASSERT_TRUE(trajectory.elements_size() == test::kExpectedNumberSegments);
}

TEST(planner, triangularTrapezoidalTrajectory) {
    const double arc_length = 1.0;
    const double curvature = 1.0;
    TrajectoryOptions options;
    constexpr int64_t kDefaultFrequency = 10;
    constexpr int64_t kNanosecondsPerSecond = 1000000000LL;
    options.set_frequency(kDefaultFrequency);
    options.set_max_velocity(1.0);
    options.set_max_acceleration(1.0);
    options.set_max_deceleration(options.max_acceleration());

    Trajectory trajectory;
    TrapezoidalTrajectory generator(options);
    ASSERT_TRUE(generator.generate(arc_length, curvature, trajectory));
    ASSERT_TRUE(trajectory.type() == TrajectoryType::TRIANGULAR);
    ASSERT_TRUE(trajectory.elements_size() > 1);
    for (int i = 1; i < trajectory.elements_size(); ++i) {
        ASSERT_GT(trajectory.elements(i).relative_time().nanos(), trajectory.elements(i - 1).relative_time().nanos());
        const int64_t end = trajectory.elements(i).relative_time().nanos();
        const int64_t start = trajectory.elements(i - 1).relative_time().nanos();
        const int64_t diff = end - start;
        EXPECT_GT(diff, 0LL);
        EXPECT_LT(std::abs(diff - (kNanosecondsPerSecond / kDefaultFrequency)), 100) << "diff = " << diff << " @ i = " << i;
        EXPECT_GE(trajectory.elements(i).linear_velocity(), 0.0);
        EXPECT_LE(trajectory.elements(i).linear_velocity(), options.max_velocity());
    }
    ASSERT_FLOAT_EQ(trajectory.elements(trajectory.elements_size() - 1).linear_velocity(), 0.0);
}

TEST(planner, trapezoidalTrajectory) {
    const float arc_length = 10.0;
    const float curvature = 1.0;

    TrajectoryOptions options;
    constexpr float kDefaultFrequency = 10.0;
    options.set_frequency(kDefaultFrequency);
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

TEST(planner, voxelIntegration) {
    using core::Point2d;

    constexpr float kPathIncrement = 0.1;
    constexpr float kPathLength = 1.0;

    Trajectory trajectory;
    auto element = trajectory.add_elements();
    std::vector<core::Point2d> sample_points;

    constexpr float kPositiveVelocity = 1.0;
    constexpr float kPositiveCurvature = 1.0;
    constexpr int kNumPoints = kPathLength / kPathIncrement;

    element->set_arclength(kPathLength);
    element->set_curvature(kPositiveCurvature);
    element->set_linear_velocity(kPositiveVelocity);
    ASSERT_TRUE(sample(trajectory.elements(0), sample_points, kPathIncrement));
    ASSERT_EQ(sample_points.size(), kNumPoints);
    for (auto point : sample_points) {
        ASSERT_GE(point.x(), 0);
        ASSERT_GE(point.y(), 0);
    }
}

TEST(planner, SBPLPlannerTolerance) {
    SBPLPlannerOptions options;
    std::unique_ptr<SBPLPlanner> planner_ptr;

    // Check that the planner type is set appropriately
    ASSERT_THROW(planner_ptr.reset(new SBPLPlanner(options)), std::runtime_error);
    options.set_planner_type(SBPLPlannerOptions::ARASTAR);
    ASSERT_NO_THROW(planner_ptr.reset(new SBPLPlanner(options)));

    // Straight forward
    double x_pos = 5.0;
    Sophus::SE3d target;
    elementsToPose(x_pos, 0.0, 0.0, 0.0, 0.0, 0.0, target);
    Path path;
    ASSERT_TRUE(planner_ptr->planTo(target, path));
    // The sbpl must be centered
    ASSERT_EQ(path.elements(0).transform().translationy(), 0);
}

TEST(planner, SBPLPlanner) {
    SBPLPlannerOptions options;
    std::unique_ptr<SBPLPlanner> planner_ptr;

    // Check that the planner type is set appropriately
    ASSERT_THROW(planner_ptr.reset(new SBPLPlanner(options)), std::runtime_error);
    options.set_planner_type(SBPLPlannerOptions::ARASTAR);
    ASSERT_NO_THROW(planner_ptr.reset(new SBPLPlanner(options)));

    // Straight forward
    {
        double x_pos = 5.0;
        Sophus::SE3d target;
        elementsToPose(x_pos, 0.0, 0.0, 0.0, 0.0, 0.0, target);
        Path path;
        ASSERT_TRUE(planner_ptr->planTo(target, path));
        for (const auto& entry : *path.mutable_elements()) {
            ASSERT_FLOAT_EQ(entry.transform().translationy(), 0.0);
            // Have a tolerance of +- a cell
            ASSERT_GE(entry.transform().translationx(), 0.0);
            ASSERT_LE(entry.transform().translationx(), x_pos);
        }
    }
    // Backwards
    {
        constexpr double x_pos = -5;
        constexpr double y_pos = 0;

        Sophus::SE3d target;
        elementsToPose(x_pos, y_pos, 0.0, 0.0, 0.0, 0.0, target);
        Path path;
        ASSERT_TRUE(planner_ptr->planTo(target, path));
        for (const auto& entry : *path.mutable_elements()) {
            ASSERT_FLOAT_EQ(entry.transform().translationy(), 0.0);
            ASSERT_LE(entry.transform().translationx(), 0.0);
            ASSERT_GT(entry.transform().translationx(), x_pos);
        }
    }
    constexpr float kGoalHeadingTolerance = 0.1;
    // Forward zero-orientation
    {
        double x = 5;
        for (double y = -2; y < 2; y += 2) {
            Sophus::SE3d target;
            elementsToPose(x, y, 0.0, 0.0, 0.0, 0.0, target);
            Path path;
            ASSERT_TRUE(planner_ptr->planTo(target, path));
            const auto& last_element = path.elements(path.elements_size() - 1);
            Sophus::SE3d pose;
            protoToPose(last_element.transform(), pose);
            double x_, y_, z_, r_, p_, q_;
            poseToElements(x_, y_, z_, r_, p_, q_, pose);
            ASSERT_LT(q_, kGoalHeadingTolerance);
            for (const auto& element : *path.mutable_elements()) {
                ASSERT_GE(element.transform().translationx(), 0.0);
            }
        }
    }
    // Rearwards zero-orientation
    {
        double x = -5;
        for (double y = -2; y < 2; y += 2) {
            Sophus::SE3d target;
            elementsToPose(x, y, 0.0, 0.0, 0.0, 0.0, target);
            Path path;
            ASSERT_TRUE(planner_ptr->planTo(target, path));
            const auto& last_element = path.elements(path.elements_size() - 1);

            Sophus::SE3d pose;
            protoToPose(last_element.transform(), pose);
            double x_, y_, z_, r_, p_, q_;
            poseToElements(x_, y_, z_, r_, p_, q_, pose);
            ASSERT_LT(q_, kGoalHeadingTolerance);
        }
    }
}

TEST(planner, stateMachineAdapter) {
    std::shared_ptr<test::DelayedPlanner> planner_ptr = std::make_shared<test::DelayedPlanner>();
    ASSERT_NE(planner_ptr.get(), nullptr);
    std::unique_ptr<StateMachineAdapter> planner_adapter_ptr;
    planner_adapter_ptr.reset(new StateMachineAdapter(planner_ptr));
    ASSERT_EQ(planner_adapter_ptr->state(), PlannerState::PLANNER_INACTIVE);

    constexpr double x_pos = 10;
    constexpr double y_pos = -10;

    Sophus::SE3d target;
    elementsToPose(x_pos, y_pos, 0.0, 0.0, 0.0, 0.0, target);
    Trajectory trajectory;

    ASSERT_TRUE(planner_adapter_ptr->planTo(target, trajectory));
    ASSERT_EQ(planner_adapter_ptr->state(), PlannerState::PLANNER_INACTIVE);
    planner_ptr->setNextReturnValue(false);
    ASSERT_FALSE(planner_adapter_ptr->planTo(target, trajectory));
    ASSERT_EQ(planner_adapter_ptr->state(), PlannerState::PLANNER_FAILED);
    planner_ptr->setNextReturnValue(true);
    ASSERT_TRUE(planner_adapter_ptr->planTo(target, trajectory));
    ASSERT_EQ(planner_adapter_ptr->state(), PlannerState::PLANNER_INACTIVE);
    // Simulate a long-running planning task
    planner_ptr->setDelay(2 * 1000);
    std::thread planning_thread([&planner_adapter_ptr, &target]() {
        Trajectory trajectory;
        planner_adapter_ptr->planTo(target, trajectory);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // Try to interrupt long-running task
    ASSERT_EQ(planner_adapter_ptr->state(), PlannerState::PLANNER_ACTIVE);
    planner_adapter_ptr->planTo(target, trajectory);
    ASSERT_EQ(planner_adapter_ptr->state(), PlannerState::PLANNER_FAILED);
    planning_thread.join();
}
} // planning

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
