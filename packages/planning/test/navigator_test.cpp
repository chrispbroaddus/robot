#include "glog/logging.h"
#include "gtest/gtest.h"

#include "common.h"
#include "packages/estimation/estimator.h"
#include "packages/planning/navigator.h"
#include "packages/planning/planner.h"
#include "packages/planning/utils.h"

namespace planning {

class InterpolatedPathPlanner : public PathPlanner {
public:
    bool planTo(const Sophus::SE3d& pose, Path& path) override {
        path.clear_elements();
        Eigen::Vector3d direction = pose.translation();
        auto length = direction.norm();
        direction[0] /= length;
        direction[1] /= length;

        constexpr double kSampleIncrement = 0.1;
        const int num_samples = static_cast<int>(length / kSampleIncrement);
        for (int i = 0; i < num_samples; ++i) {
            auto point = direction * (i * kSampleIncrement);
            auto element = path.add_elements();
            element->mutable_transform()->set_translationx(point[0]);
            element->mutable_transform()->set_translationy(point[1]);
        }
        return (path.elements_size() > 0);
    }
};

} // planning

namespace estimation {
class InterpolatedEstimator : public Estimator {
public:
    InterpolatedEstimator(const Sophus::SE3d& target, const Sophus::SE3d& start) {
        Eigen::Vector3d start_position = start.translation();
        Eigen::Vector3d end_position = target.translation();
        Eigen::Vector3d direction = (end_position - start_position);
        auto length = direction.norm();
        direction[0] /= length;
        direction[1] /= length;

        constexpr double kSampleIncrement = 0.1;
        const int num_samples = static_cast<int>(length / kSampleIncrement);
        for (int i = 0; i < num_samples; ++i) {
            Eigen::Vector3d position = start_position + (direction * (i * kSampleIncrement));
            auto orientation = Eigen::Quaterniond{ 1, 0, 0, 0 };
            Sophus::SE3d pose(orientation, position);
            estimation::State state;
            state.m_time = std::chrono::nanoseconds{ i };
            state.m_pose = pose;
            m_stateHistory.emplace_back(state);
        }
    }

    std::deque<State> states() override {
        m_stateHistory.pop_front();
        return m_stateHistory;
    }
};

class NullEstimator : public Estimator {
public:
    NullEstimator(__attribute__((unused)) const Sophus::SE3d& target, __attribute__((unused)) const Sophus::SE3d& start) {
        Eigen::Vector3d position = Eigen::Vector3d::Zero();
        auto orientation = Eigen::Quaterniond{ 1, 0, 0, 0 };
        Sophus::SE3d pose(orientation, position);
        estimation::State state;
        state.m_time = std::chrono::nanoseconds{ 0 };
        state.m_pose = pose;
        m_stateHistory.emplace_back(state);
    }
};

} // estimation

TEST(planner, navigatorStateMachine) { ASSERT_TRUE(true); }

TEST(planner, decisionFunctionTest) {
    using planning::EuclideanDistanceDecisionFunction;

    constexpr double kDefaultDecisionDistance = 0.5;
    EuclideanDistanceDecisionFunction function(kDefaultDecisionDistance);

    Sophus::SE3d start{ Eigen::Quaterniond{ 1, 0, 0, 0 }, Eigen::Vector3d{ 0, 0, 0 } };
    Sophus::SE3d goal{ Eigen::Quaterniond{ 1, 0, 0, 0 }, Eigen::Vector3d{ 0, 0, 0 } };

    for (double x = 0.0; x < 5.0; x += 0.2) {
        for (double y = 0.0; y < 5.0; y += 0.2) {
            goal.translation()[0] = x;
            goal.translation()[1] = y;
            auto dist = std::sqrt(std::pow(x, 2) + std::pow(y, 2));
            if (dist > kDefaultDecisionDistance) {
                ASSERT_FALSE(function(start, goal));
            } else {
                ASSERT_TRUE(function(start, goal));
            }
        }
    }
}
TEST(planner, defaultPointAndGoNavigator) {
    using planning::elementsToPose;
    using planning::Comms;
    using planning::Navigator;
    using planning::PathPlanner;
    using planning::TrajectoryPlanner;
    using planning::test::MockComms;
    using planning::test::DefaultTrajectoryPlanner;
    using planning::test::DefaultPathPlanner;

    std::unique_ptr<TrajectoryPlanner> trajectory_planner_ptr(new DefaultTrajectoryPlanner);
    std::unique_ptr<PathPlanner> path_planner_ptr(new DefaultPathPlanner);
    std::unique_ptr<Comms> comms_ptr(new MockComms);

    using planning::DefaultPointAndGoNavigator;
    std::unique_ptr<Navigator> navigator_ptr;
    navigator_ptr.reset(new DefaultPointAndGoNavigator(std::move(comms_ptr), std::move(trajectory_planner_ptr)));
    ASSERT_NE(navigator_ptr.get(), nullptr);
    Sophus::SE3d target;
    elementsToPose(1.0, 1.0, 0.0, 0.0, 0.0, 0.0, target);
    auto future = navigator_ptr->navigateTo(target);
    ASSERT_TRUE(future.get());
}

TEST(planner, waypointNavigator) {
    using planning::elementsToPose;
    using planning::Comms;
    using planning::Navigator;
    using planning::TrajectoryPlanner;
    using planning::PathPlanner;
    using planning::InterpolatedPathPlanner;
    using planning::GoalDecisionFunction;
    using planning::EuclideanDistanceDecisionFunction;
    using planning::test::MockComms;
    using planning::test::DefaultTrajectoryPlanner;
    using planning::NavigatorOptions;
    using estimation::Estimator;
    using estimation::NullEstimator;
    using estimation::InterpolatedEstimator;

    Sophus::SE3d target;
    double goal_x = 5.0, goal_y = 2.0;
    elementsToPose(goal_x, goal_y, 0.0, 0.0, 0.0, 0.0, target);
    Sophus::SE3d start;
    double start_x = -5.0, start_y = -2.0;
    elementsToPose(start_x, start_y, 0.0, 0.0, 0.0, 0.0, start);

    std::unique_ptr<TrajectoryPlanner> trajectory_planner_ptr;
    std::unique_ptr<PathPlanner> path_planner_ptr;
    std::shared_ptr<Estimator> estimator_ptr;
    std::unique_ptr<GoalDecisionFunction> decision_ptr;
    std::unique_ptr<Navigator> navigator_ptr;
    std::unique_ptr<Comms> comms_ptr;

    NavigatorOptions options;
    options.set_frequency(10);

    using planning::WaypointNavigator;
    {
        trajectory_planner_ptr.reset(new DefaultTrajectoryPlanner);
        path_planner_ptr.reset(new InterpolatedPathPlanner);
        estimator_ptr.reset(new InterpolatedEstimator(target, start));
        decision_ptr.reset(new EuclideanDistanceDecisionFunction);
        comms_ptr.reset(new MockComms);
        navigator_ptr.reset(new WaypointNavigator(options, std::move(comms_ptr), std::move(trajectory_planner_ptr),
            std::move(path_planner_ptr), estimator_ptr, std::move(decision_ptr)));
        auto future = navigator_ptr->navigateTo(target);
        ASSERT_TRUE(future.get());
    }
    {
        trajectory_planner_ptr.reset(new DefaultTrajectoryPlanner);
        path_planner_ptr.reset(new InterpolatedPathPlanner);
        estimator_ptr.reset(new NullEstimator(target, start));
        decision_ptr.reset(new EuclideanDistanceDecisionFunction);
        comms_ptr.reset(new MockComms);
        navigator_ptr.reset(new WaypointNavigator(options, std::move(comms_ptr), std::move(trajectory_planner_ptr),
            std::move(path_planner_ptr), estimator_ptr, std::move(decision_ptr)));
        auto future = navigator_ptr->navigateTo(target);
        navigator_ptr->stop();
        ASSERT_FALSE(future.get());
    }
}

TEST(planner, fallbackPlanner) {
    using planning::ArcPlanner;
    using planning::ArcPlannerOptions;
    using planning::elementsToPose;
    using planning::FallbackPlanner;
    using planning::Trajectory;
    using planning::Trajectory;
    using planning::SBPLPlanner;
    using planning::SBPLPlannerOptions;
    using planning::Path;
    using planning::TrajectoryGenerator;
    using planning::TrajectoryOptions;
    using planning::loadDefaultTrajectoryOptions;
    using planning::loadDefaultArcPlannerOptions;
    using planning::SquareTrajectory;

    SBPLPlannerOptions path_options;
    std::unique_ptr<SBPLPlanner> planner_ptr;
    path_options.set_planner_type(SBPLPlannerOptions::ARASTAR);
    ASSERT_NO_THROW(planner_ptr.reset(new SBPLPlanner(path_options)));

    TrajectoryOptions trajectory_options = loadDefaultTrajectoryOptions();
    std::unique_ptr<TrajectoryGenerator> generator_ptr(new SquareTrajectory(trajectory_options));
    std::unique_ptr<ArcPlanner> trajectory_planner_ptr;
    ArcPlannerOptions arc_planner_options = planning::loadDefaultArcPlannerOptions();
    arc_planner_options.set_max_curvature(std::numeric_limits<float>::infinity());
    trajectory_planner_ptr.reset(new ArcPlanner(arc_planner_options, std::move(generator_ptr)));
    ASSERT_NE(trajectory_planner_ptr.get(), nullptr);

    Sophus::SE3d target;
    constexpr double x_pos = 5.0;
    constexpr double y_pos = 5.0;
    elementsToPose(x_pos, y_pos, 0.0, 0.0, 0.0, 0.0, target);

    Path path;
    ASSERT_TRUE(planner_ptr->planTo(target, path));
    FallbackPlanner fb_planner;
    Trajectory trajectory;
    auto sampled_path = [](const Path& input) -> Path {
        Path output;
        constexpr double kSampleDistance = 0.5;
        double accumulatedDistance = 0.0;
        for (int i = 1; i < input.elements_size(); ++i) {
            Eigen::Vector3d current{ input.elements(i).transform().translationx(), input.elements(i).transform().translationy(), 0 };

            Eigen::Vector3d previous{ input.elements(i - 1).transform().translationx(), input.elements(i - 1).transform().translationy(),
                0 };

            accumulatedDistance += (current - previous).norm();
            if (accumulatedDistance > kSampleDistance) {
                auto* element = output.add_elements();
                *element = input.elements(i);
                accumulatedDistance = 0;
            }
        }
        CHECK(output.elements_size() > 0);
        CHECK(output.elements_size() < input.elements_size());
        return output;
    }(path);
    ASSERT_TRUE(fb_planner.planAcross(sampled_path, *trajectory_planner_ptr, trajectory));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
