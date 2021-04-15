#include "glog/logging.h"
#include "gtest/gtest.h"

#include <chrono>
#include <thread>

#include "packages/estimation/estimator.h"
#include "packages/executor/executor.h"
#include "packages/executor/proto/executor_options.pb.h"
#include "packages/executor/proto_helpers.h"
#include "packages/perception/perception.h"
#include "packages/planning/planner.h"
#include "packages/planning/proto/trajectory_planner_options.pb.h"
#include "packages/planning/test/common.h"
#include "packages/planning/utils.h"
#include "packages/teleop/proto/backend_message.pb.h"

namespace executor {

TEST(executor, configuration) { ExecutorOptions executor_options = loadDefaultExecutorOptions(); }

TEST(executor, instantiate) {
    using estimation::test::DefaultEstimator;
    using estimation::Estimator;
    using planning::Comms;
    using planning::TrajectoryPlanner;
    using planning::test::DefaultTrajectoryPlanner;
    using planning::test::MockComms;
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::test::DefaultPerception;

    ExecutorOptions executor_options = loadDefaultExecutorOptions();

    std::unique_ptr<Estimator> estimator_ptr(new DefaultEstimator);
    std::unique_ptr<TrajectoryPlanner> planner_ptr(new DefaultTrajectoryPlanner);
    std::unique_ptr<Perception> perception_ptr(new DefaultPerception(PerceptionOptions()));
    std::unique_ptr<Comms> comms_ptr(new MockComms);
    std::unique_ptr<Executor> executor_ptr;
    executor_ptr.reset(
        new Executor(executor_options, std::move(estimator_ptr), std::move(planner_ptr), std::move(perception_ptr), std::move(comms_ptr)));
    ASSERT_NE(executor_ptr.get(), nullptr);
    core::Point3d target;
    ASSERT_TRUE(executor_ptr->execute(target));
}

TEST(executor, query) {
    using estimation::Estimator;
    using estimation::test::DefaultEstimator;
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::test::DefaultPerception;
    using planning::Comms;
    using planning::TrajectoryPlanner;
    using planning::test::DefaultTrajectoryPlanner;
    using planning::test::DefaultPlannerStateMachine;
    using planning::test::MockComms;

    ExecutorOptions executor_options = loadDefaultExecutorOptions();

    std::unique_ptr<Estimator> estimator_ptr(new DefaultEstimator);
    std::shared_ptr<TrajectoryPlanner> planner_ptr(new DefaultTrajectoryPlanner);

    std::unique_ptr<DefaultPlannerStateMachine> planner_adapter_ptr;
    planner_adapter_ptr.reset(new DefaultPlannerStateMachine(planner_ptr));

    std::unique_ptr<Perception> perception_ptr(new DefaultPerception(PerceptionOptions()));
    std::unique_ptr<Comms> comms_ptr(new MockComms);
    std::unique_ptr<Executor> executor_ptr;
    executor_ptr.reset(new Executor(
        executor_options, std::move(estimator_ptr), std::move(planner_adapter_ptr), std::move(perception_ptr), std::move(comms_ptr)));
    ASSERT_NE(executor_ptr.get(), nullptr);
    core::Point3d target;
    target.set_x(1.0);
    target.set_y(2.0);
    ASSERT_TRUE(executor_ptr->execute(target));
}

TEST(executor, timedQuery) {
    using estimation::Estimator;
    using estimation::test::DefaultEstimator;
    using planning::StateMachineAdapter;
    using planning::Comms;
    using planning::TrajectoryPlanner;
    using planning::TrajectoryPlannerOptions;
    using planning::test::TimedDummyPlanner;
    using planning::test::MockComms;
    using perception::PerceptionOptions;

    // Basic time-based execution strategy
    ExecutorOptions executor_options;
    executor_options.set_policy(ExecutionPolicy::TIME);
    executor_options.set_plannerdelayms(10);

    constexpr float kTestDuration = 2.0;

    std::unique_ptr<Estimator> estimator_ptr(new DefaultEstimator);
    std::shared_ptr<TrajectoryPlanner> planner_ptr = std::make_shared<TimedDummyPlanner>(TrajectoryPlannerOptions(), kTestDuration);
    std::unique_ptr<StateMachineAdapter> planner_adapter_ptr;
    planner_adapter_ptr.reset(new StateMachineAdapter(planner_ptr));

    std::unique_ptr<perception::TerrainRepresentation> terrain;
    std::unique_ptr<perception::Perception> perception_ptr(new perception::Perception(PerceptionOptions(), std::move(terrain)));
    std::unique_ptr<Comms> comms_ptr(new MockComms);
    std::unique_ptr<Executor> executor_ptr;
    executor_ptr.reset(new Executor(
        executor_options, std::move(estimator_ptr), std::move(planner_adapter_ptr), std::move(perception_ptr), std::move(comms_ptr)));
    ASSERT_NE(executor_ptr.get(), nullptr);
    core::Point3d target;
    target.set_x(1.0);
    target.set_y(2.0);
    using planning::PlannerState;

    CHECK(executor_ptr->vehicleState() == VehicleState::VEHICLE_STOPPED);
    ASSERT_TRUE(executor_ptr->execute(target));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CHECK(executor_ptr->vehicleState() == VehicleState::VEHICLE_MOVING);
    std::this_thread::sleep_for(std::chrono::seconds(static_cast<int>(kTestDuration)));
    CHECK(executor_ptr->vehicleState() == VehicleState::VEHICLE_STOPPED);
}

TEST(executor, timedJoystickCommand) {
    using estimation::Estimator;
    using estimation::test::DefaultEstimator;
    using estimation::EstimatorOptions;
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::test::DefaultPerception;
    using planning::StateMachineAdapter;
    using planning::Comms;
    using planning::TrajectoryPlanner;
    using planning::test::TimedDummyPlanner;
    using planning::TrajectoryPlannerOptions;
    using planning::test::MockComms;
    using planning::test::TimedDummyPlanner;

    // Basic time-based execution strategy
    ExecutorOptions executor_options;
    executor_options.set_policy(ExecutionPolicy::TIME);

    std::unique_ptr<Estimator> estimator_ptr(new DefaultEstimator);
    std::shared_ptr<TrajectoryPlanner> planner_ptr = std::make_shared<TimedDummyPlanner>(TrajectoryPlannerOptions(), 4.0);
    std::unique_ptr<StateMachineAdapter> planner_adapter_ptr;
    planner_adapter_ptr.reset(new StateMachineAdapter(planner_ptr));
    std::unique_ptr<Perception> perception_ptr(new DefaultPerception(PerceptionOptions()));
    std::unique_ptr<Comms> comms_ptr(new MockComms);
    std::unique_ptr<Executor> executor_ptr;
    executor_ptr.reset(new Executor(
        executor_options, std::move(estimator_ptr), std::move(planner_adapter_ptr), std::move(perception_ptr), std::move(comms_ptr)));
    CHECK(executor_ptr->vehicleState() == VehicleState::VEHICLE_STOPPED);
    teleop::JoystickCommand command;
    ASSERT_FALSE(executor_ptr->execute(command));
}

TEST(executor, timedInteruptCommand) {
    using estimation::Estimator;
    using estimation::EstimatorOptions;
    using estimation::test::DefaultEstimator;
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::test::DefaultPerception;
    using planning::StateMachineAdapter;
    using planning::Comms;
    using planning::TrajectoryPlanner;
    using planning::test::TimedDummyPlanner;
    using planning::TrajectoryPlannerOptions;
    using planning::test::MockComms;
    using planning::test::TimedDummyPlanner;

    ExecutorOptions executor_options;
    executor_options.set_policy(ExecutionPolicy::TIME);
    executor_options.set_plannerdelayms(10);

    std::unique_ptr<Estimator> estimator_ptr(new DefaultEstimator);
    std::shared_ptr<TrajectoryPlanner> planner_ptr = std::make_shared<TimedDummyPlanner>(TrajectoryPlannerOptions(), 4.0);
    std::unique_ptr<StateMachineAdapter> planner_adapter_ptr;
    planner_adapter_ptr.reset(new StateMachineAdapter(planner_ptr));

    std::unique_ptr<Perception> perception_ptr(new DefaultPerception(PerceptionOptions()));
    std::unique_ptr<Comms> comms_ptr(new MockComms);
    std::unique_ptr<Executor> executor_ptr;
    executor_ptr.reset(new Executor(
        executor_options, std::move(estimator_ptr), std::move(planner_adapter_ptr), std::move(perception_ptr), std::move(comms_ptr)));
    CHECK(executor_ptr->vehicleState() == VehicleState::VEHICLE_STOPPED);
    core::Point3d target;
    target.set_x(2.0);
    target.set_y(1.0);
    ASSERT_TRUE(executor_ptr->execute(target));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_TRUE(executor_ptr->execute(target));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_TRUE(executor_ptr->execute(target));
    ASSERT_TRUE(executor_ptr->wait());
    CHECK(executor_ptr->vehicleState() == VehicleState::VEHICLE_STOPPED);

    teleop::JoystickCommand command;
    ASSERT_TRUE(executor_ptr->execute(target));
    ASSERT_FALSE(executor_ptr->execute(command));
}

TEST(executor, perceptionIntegration) {
    using estimation::Estimator;
    using estimation::EstimatorOptions;
    using planning::Comms;
    using planning::TrajectoryPlanner;
    using planning::test::TimedDummyPlanner;
    using planning::TrajectoryPlannerOptions;
    using planning::StateMachineAdapter;
    using estimation::Estimator;
    using estimation::test::DefaultEstimator;
    using perception::Perception;
    using perception::test::SafetyPerception;
    using perception::PerceptionOptions;
    using planning::Comms;
    using planning::TrajectoryPlanner;
    using planning::TrajectoryPlannerOptions;
    using planning::StateMachineAdapter;
    using planning::test::MockComms;
    using planning::test::TimedDummyPlanner;

    ExecutorOptions executor_options;
    executor_options.set_policy(ExecutionPolicy::TIME);
    executor_options.set_plannerdelayms(10);

    std::unique_ptr<Estimator> estimator_ptr(new DefaultEstimator);
    std::shared_ptr<TrajectoryPlanner> planner_ptr = std::make_shared<TimedDummyPlanner>(TrajectoryPlannerOptions(), 4.0);
    std::unique_ptr<StateMachineAdapter> planner_adapter_ptr;
    planner_adapter_ptr.reset(new StateMachineAdapter(planner_ptr));

    std::unique_ptr<Perception> perception_ptr(new SafetyPerception(PerceptionOptions()));
    std::unique_ptr<Comms> comms_ptr(new MockComms);
    std::unique_ptr<Executor> executor_ptr;
    executor_ptr.reset(new Executor(
        executor_options, std::move(estimator_ptr), std::move(planner_adapter_ptr), std::move(perception_ptr), std::move(comms_ptr)));
    core::Point3d target;
    target.set_x(10.0);
    target.set_y(0.0);
    ASSERT_TRUE(executor_ptr->vehicleState() != VehicleState::VEHICLE_FAILURE);
    ASSERT_TRUE(executor_ptr->execute(target));
    ASSERT_TRUE(executor_ptr->wait());
}

TEST(executor, logging) {
    using estimation::Estimator;
    using estimation::EstimatorOptions;
    using estimation::test::DefaultEstimator;
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::test::DefaultPerception;
    using planning::TrajectoryPlannerOptions;
    using planning::TrajectoryPlanner;
    using planning::StateMachineAdapter;
    using planning::FilenameCreator;
    using planning::test::MockComms;
    using planning::test::TimedDummyPlanner;

    FilenameCreator creator("executor");
    const auto filename = creator();

    ExecutorOptions executor_options;
    executor_options.set_logfile(filename);
    executor_options.set_policy(ExecutionPolicy::TIME);
    executor_options.set_plannerdelayms(10);

    std::unique_ptr<Estimator> estimator_ptr(new DefaultEstimator);
    std::shared_ptr<TrajectoryPlanner> planner_ptr = std::make_shared<TimedDummyPlanner>(TrajectoryPlannerOptions(), 4.0);
    std::unique_ptr<StateMachineAdapter> planner_adapter_ptr;
    planner_adapter_ptr.reset(new StateMachineAdapter(planner_ptr));

    std::unique_ptr<Perception> perception_ptr(new DefaultPerception(PerceptionOptions()));
    std::unique_ptr<MockComms> comms_ptr(new MockComms);
    std::unique_ptr<Executor> executor_ptr;
    executor_ptr.reset(new Executor(
        executor_options, std::move(estimator_ptr), std::move(planner_adapter_ptr), std::move(perception_ptr), std::move(comms_ptr)));
    core::Point3d target;
    target.set_x(2.0);
    target.set_y(2.0);
    ASSERT_TRUE(executor_ptr->execute(target));
    ASSERT_TRUE(executor_ptr->wait());
    using planning::readLog;
    std::deque<LogEntry> log_entries;
    ASSERT_TRUE(readLog(filename, log_entries));
    ASSERT_TRUE(!log_entries.empty());
}

TEST(executor, pathQueryTest) {
    using estimation::Estimator;
    using estimation::EstimatorOptions;
    using planning::StateMachineAdapter;
    using planning::TrajectoryPlanner;
    using estimation::test::DefaultEstimator;
    using planning::TrajectoryPlannerOptions;
    using planning::test::TimedDummyPlanner;
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::test::DefaultPerception;
    using planning::StateMachineAdapter;
    using planning::TrajectoryPlanner;
    using planning::TrajectoryPlannerOptions;
    using planning::test::MockComms;
    using planning::test::TimedDummyPlanner;

    ExecutorOptions executor_options;
    executor_options.set_policy(ExecutionPolicy::TIME);
    executor_options.set_plannerdelayms(10);

    std::unique_ptr<Estimator> estimator_ptr(new DefaultEstimator);
    std::shared_ptr<TrajectoryPlanner> planner_ptr = std::make_shared<TimedDummyPlanner>(TrajectoryPlannerOptions(), 4.0);
    std::unique_ptr<StateMachineAdapter> planner_adapter_ptr;
    planner_adapter_ptr.reset(new StateMachineAdapter(planner_ptr));
    std::unique_ptr<Perception> perception_ptr(new DefaultPerception(PerceptionOptions()));
    std::unique_ptr<MockComms> comms_ptr(new MockComms);
    std::unique_ptr<Executor> executor_ptr;
    executor_ptr.reset(new Executor(
        executor_options, std::move(estimator_ptr), std::move(planner_adapter_ptr), std::move(perception_ptr), std::move(comms_ptr)));
    core::Point3d target;
    target.set_x(10.0);
    target.set_y(0.0);
    ASSERT_TRUE(executor_ptr->vehicleState() != VehicleState::VEHICLE_FAILURE);
    ASSERT_TRUE(executor_ptr->execute(target));
    ASSERT_TRUE(executor_ptr->wait());
}

TEST(executor, invalidPointAndGoTimestamps) {
    using estimation::Estimator;
    using estimation::test::DefaultEstimator;
    using perception::Perception;
    using perception::test::DefaultPerception;
    using perception::PerceptionOptions;
    using planning::StateMachineAdapter;
    using planning::Comms;
    using planning::TrajectoryPlanner;
    using planning::TrajectoryPlannerOptions;
    using planning::test::TimedDummyPlanner;
    using planning::test::MockComms;

    // Basic time-based execution strategy
    ExecutorOptions executor_options;
    executor_options.set_policy(ExecutionPolicy::TIME);
    executor_options.set_plannerdelayms(10);
    executor_options.set_offsettolerancems(1500);

    std::unique_ptr<Estimator> estimator_ptr(new DefaultEstimator);
    std::shared_ptr<TrajectoryPlanner> planner_ptr = std::make_shared<TimedDummyPlanner>(TrajectoryPlannerOptions(), 4.0);
    std::unique_ptr<StateMachineAdapter> planner_adapter_ptr;
    planner_adapter_ptr.reset(new StateMachineAdapter(planner_ptr));
    std::unique_ptr<Perception> perception_ptr(new DefaultPerception(PerceptionOptions()));
    std::unique_ptr<Comms> comms_ptr(new MockComms);
    std::unique_ptr<Executor> executor_ptr;
    executor_ptr.reset(new Executor(
        executor_options, std::move(estimator_ptr), std::move(planner_adapter_ptr), std::move(perception_ptr), std::move(comms_ptr)));
    CHECK(executor_ptr->vehicleState() == VehicleState::VEHICLE_STOPPED);

    core::SystemTimestamp timestamp;
    using core::chrono::gps::wallClockInNanoseconds;
    {
        auto futureTimestamp = wallClockInNanoseconds() + std::chrono::nanoseconds(static_cast<int>(2 * 1e9));
        timestamp.set_nanos(futureTimestamp.count());
        teleop::PointAndGoCommand command;
        *command.mutable_operatortimestamp() = timestamp;
        auto result = executor_ptr->execute(command);
        ASSERT_EQ(result.response().code(), ExecutorResponse::COMMAND_TOO_FAR_IN_FUTURE);
    }
    {
        auto pastTimestamp = wallClockInNanoseconds() - std::chrono::nanoseconds(static_cast<int>(2 * 1e9));
        timestamp.set_nanos(pastTimestamp.count());
        teleop::PointAndGoCommand command;
        *command.mutable_operatortimestamp() = timestamp;
        auto result = executor_ptr->execute(command);
        ASSERT_EQ(result.response().code(), ExecutorResponse::COMMAND_TOO_FAR_IN_PAST);
    }
    {
        auto validTimestamp = wallClockInNanoseconds() - std::chrono::nanoseconds(static_cast<int>(1 * 1e8));
        timestamp.set_nanos(validTimestamp.count());
        teleop::PointAndGoCommand command;
        *command.mutable_operatortimestamp() = timestamp;
        auto result = executor_ptr->execute(command);
        ASSERT_EQ(result.response().code(), ExecutorResponse::VALID);
    }
}
} // executor

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
