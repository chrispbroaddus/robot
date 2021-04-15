#include "packages/planning/planner.h"
#include "packages/planning/vcu_state_machine.h"
#include "packages/planning/vcu_trajectory_check.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

using std::chrono::nanoseconds;

using core::chrono::gps::wallClockInNanoseconds;
using hal::VCUArcDriveSegment;
using hal::VCUTurnInPlaceSegment;
using hal::VCUCommandEnvelope;
using hal::VCUIdleCommand;
using hal::VCUTrajectoryCommand;
using planning::commandValidate;

using namespace planning;

TEST(vcuInterop, invalidSequenceNumber) {
    VCUTrajectoryCommand trajectory;
    VCUCommandEnvelope envelope;
    *envelope.mutable_trajectorycommand() = trajectory;
    int errorCode = -1;
    ASSERT_FALSE(commandValidate(envelope, errorCode));
    ASSERT_EQ(errorCode, 2);
}

TEST(vcuInterop, nonTrajectoryCommand) {
    VCUIdleCommand idle;
    VCUCommandEnvelope envelope;
    *envelope.mutable_idlecommand() = idle;
    envelope.set_sequencenumber(1);
    int errorCode = -1;
    ASSERT_TRUE(commandValidate(envelope, errorCode));
    ASSERT_EQ(errorCode, 0);
}

TEST(vcuInterop, trajectoryCommandInvalidSize) {
    VCUTrajectoryCommand trajectory;
    VCUCommandEnvelope envelope;
    *envelope.mutable_trajectorycommand() = trajectory;
    envelope.set_sequencenumber(1);
    int errorCode = -1;
    ASSERT_FALSE(commandValidate(envelope, errorCode));
    ASSERT_EQ(errorCode, 4);
}

TEST(vcuInterop, trajectoryCommandInvalidTerminalVelocity) {
    VCUTrajectoryCommand trajectory;

    VCUArcDriveSegment arcDriveCommand;
    arcDriveCommand.set_linearvelocitymeterspersecond(1.0);
    auto segment = trajectory.add_segments();
    *segment->mutable_arcdrive() = arcDriveCommand;

    VCUCommandEnvelope envelope;
    *envelope.mutable_trajectorycommand() = trajectory;
    envelope.set_sequencenumber(1);
    int errorCode = -1;
    ASSERT_FALSE(commandValidate(envelope, errorCode));
    ASSERT_EQ(errorCode, 5);
}

TEST(vcuInterop, trajectoryCommandInvalidPositiveCurvature) {
    VCUTrajectoryCommand trajectory;

    VCUArcDriveSegment arcDriveCommand;
    arcDriveCommand.set_linearvelocitymeterspersecond(0.0);
    arcDriveCommand.set_curvatureinversemeters(planning::PlannerConstants<float>::kCurvatureLimit + 1);
    auto segment = trajectory.add_segments();
    *segment->mutable_arcdrive() = arcDriveCommand;

    VCUCommandEnvelope envelope;
    *envelope.mutable_trajectorycommand() = trajectory;
    envelope.set_sequencenumber(1);
    int errorCode = -1;
    ASSERT_FALSE(commandValidate(envelope, errorCode));
    ASSERT_EQ(errorCode, 6);
}

TEST(vcuInterop, trajectoryCommandInvalidNegativeCurvature) {
    VCUTrajectoryCommand trajectory;

    VCUArcDriveSegment arcDriveCommand;
    arcDriveCommand.set_linearvelocitymeterspersecond(0.0);
    arcDriveCommand.set_curvatureinversemeters(-planning::PlannerConstants<float>::kCurvatureLimit - 1);
    auto segment = trajectory.add_segments();
    *segment->mutable_arcdrive() = arcDriveCommand;

    VCUCommandEnvelope envelope;
    *envelope.mutable_trajectorycommand() = trajectory;
    envelope.set_sequencenumber(1);
    int errorCode = -1;
    ASSERT_FALSE(commandValidate(envelope, errorCode));
    ASSERT_EQ(errorCode, 6);
}

TEST(vcuInterop, trajectoryCommandInvalidPositiveVelocity) {
    VCUTrajectoryCommand trajectory;

    VCUArcDriveSegment arcDriveCommand;
    arcDriveCommand.set_linearvelocitymeterspersecond(+planning::PlannerConstants<float>::kHardSpeedLimit + 1);
    arcDriveCommand.set_curvatureinversemeters(0.0);

    auto segment = trajectory.add_segments();
    *segment->mutable_arcdrive() = arcDriveCommand;

    // Add in a "stop"
    trajectory.add_segments();

    VCUCommandEnvelope envelope;
    *envelope.mutable_trajectorycommand() = trajectory;
    envelope.set_sequencenumber(1);
    int errorCode = -1;
    ASSERT_FALSE(commandValidate(envelope, errorCode));
    ASSERT_EQ(errorCode, 6);
}

TEST(vcuInterop, trajectoryCommandInvalidNegativeVelocity) {
    VCUTrajectoryCommand trajectory;

    VCUArcDriveSegment arcDriveCommand;
    arcDriveCommand.set_linearvelocitymeterspersecond(-planning::PlannerConstants<float>::kHardSpeedLimit - 1);
    arcDriveCommand.set_curvatureinversemeters(0.0);
    auto segment = trajectory.add_segments();
    *segment->mutable_arcdrive() = arcDriveCommand;

    // Add in a "stop"
    trajectory.add_segments();

    VCUCommandEnvelope envelope;
    *envelope.mutable_trajectorycommand() = trajectory;
    envelope.set_sequencenumber(1);
    int errorCode = -1;
    ASSERT_FALSE(commandValidate(envelope, errorCode));
    ASSERT_EQ(errorCode, 6);
}

TEST(vcuInterop, trajectoryCommandInPast) {
    VCUTrajectoryCommand trajectory;

    // Add in a "traverse" segment
    trajectory.add_segments();
    // Add in a "stop" segment
    trajectory.add_segments();

    VCUCommandEnvelope envelope;
    *envelope.mutable_trajectorycommand() = trajectory;
    envelope.set_sequencenumber(1);
    int errorCode = -1;
    ASSERT_FALSE(commandValidate(envelope, errorCode));
    ASSERT_EQ(errorCode, 7);
}

TEST(vcuInterop, trajectoryCommandNonChronological) {
    VCUTrajectoryCommand trajectory;

    auto now = wallClockInNanoseconds();

    constexpr int kMessageOffset = static_cast<int>(5e8);
    auto then = std::chrono::duration_cast<nanoseconds>(now + nanoseconds(kMessageOffset));

    // Add in a "traverse" segment
    auto traverse = trajectory.add_segments();
    core::SystemTimestamp targetstarttime;
    targetstarttime.set_nanos(then.count());
    *traverse->mutable_targetstarttime() = targetstarttime;

    // Add in a "stop" segment
    trajectory.add_segments();

    VCUCommandEnvelope envelope;
    *envelope.mutable_trajectorycommand() = trajectory;
    envelope.set_sequencenumber(1);
    int errorCode = -1;
    ASSERT_FALSE(commandValidate(envelope, errorCode));
    ASSERT_EQ(errorCode, 8);
}

TEST(vcuInterop, validTrajectoryCommand) {
    VCUTrajectoryCommand trajectory;

    auto now = wallClockInNanoseconds();
    constexpr int kMessageOffset = static_cast<int>(5e8);

    // Add in a "traverse" segment
    auto traverse = trajectory.add_segments();
    core::SystemTimestamp targetstarttime;
    auto offset = std::chrono::duration_cast<nanoseconds>(now + nanoseconds(kMessageOffset));
    targetstarttime.set_nanos(offset.count());
    *traverse->mutable_targetstarttime() = targetstarttime;

    // Add in a "stop" segment
    auto stop = trajectory.add_segments();
    offset = std::chrono::duration_cast<nanoseconds>(offset + nanoseconds(kMessageOffset));
    targetstarttime.set_nanos(offset.count());
    *stop->mutable_targetstarttime() = targetstarttime;

    VCUCommandEnvelope envelope;
    *envelope.mutable_trajectorycommand() = trajectory;
    envelope.set_sequencenumber(1);
    int errorCode = -1;
    ASSERT_TRUE(commandValidate(envelope, errorCode));
    ASSERT_EQ(errorCode, 0);
}

TEST(vcuInterop, vcuStateMachine) {
    using planning::interop::ZippyStateMachineDriver;
    VCUTrajectoryCommand trajectory;

    VCUArcDriveSegment arcDriveCommand;
    arcDriveCommand.set_linearvelocitymeterspersecond(1.0);

    VCUTurnInPlaceSegment turnInPlace;
    turnInPlace.set_rotationalvelocityradianspersecond(1.0);

    // TODO (ian):
    // Flesh out the state-machine tests
    // https://zippyai.atlassian.net/browse/AUTO-712
    {
        // Inititalize -> ArcDrive
        auto segment = trajectory.add_segments();
        *segment->mutable_arcdrive() = arcDriveCommand;
        ZippyStateMachineDriver driver;
        ASSERT_NO_THROW(driver.validate(trajectory));

        // ArcDrive -> ArcDrive
        segment = trajectory.add_segments();
        *segment->mutable_arcdrive() = arcDriveCommand;
        ASSERT_NO_THROW(driver.validate(trajectory));

        // ArcDrive -> TurnInPlace
        segment = trajectory.add_segments();
        *segment->mutable_turninplace() = turnInPlace;
        ASSERT_THROW(driver.validate(trajectory), std::runtime_error);
    }
}

TEST(vcuInterop, vcuTimingCompatibility) {
    using core::Duration;
    using planning::toDuration;
    using planning::Trajectory;
    using planning::relativeTrajectoryToAbsolute;

    Trajectory relative, absolute;
    auto traverse = relative.add_elements(); // Traverse
    *traverse->mutable_relative_time() = toDuration(.1);
    relative.add_elements(); // Stop

    /*
     *Send a trajectory with zero offset. This will induce a failure on the AI
     *  side, as the trajectory-checker will perceive the message as being in
     *  the past.
     */
    relativeTrajectoryToAbsolute(relative, std::chrono::nanoseconds{ 0 }, absolute);

    hal::VCUTrajectoryCommand command;
    VCUCommandEnvelope envelope;

    plannerTrajectoryToVCUTrajectory(absolute, command);
    int errorCode = -1;
    *envelope.mutable_trajectorycommand() = command;
    envelope.set_sequencenumber(1);
    ASSERT_FALSE(commandValidate(envelope, errorCode));
    ASSERT_EQ(errorCode, 7);

    /*
     *Send a trajectory with the default offset - this must work
     */
    relativeTrajectoryToAbsolute(relative, std::chrono::nanoseconds(kDefaultPlannerToleranceNs), absolute);
    plannerTrajectoryToVCUTrajectory(absolute, command);
    *envelope.mutable_trajectorycommand() = command;
    ASSERT_TRUE(commandValidate(envelope, errorCode));

    /*
     *Create a trajectory, and sleep for half of the planner tolerance - this
     *  must also work
     */
    relativeTrajectoryToAbsolute(relative, std::chrono::nanoseconds(kDefaultPlannerToleranceNs), absolute);
    plannerTrajectoryToVCUTrajectory(absolute, command);
    *envelope.mutable_trajectorycommand() = command;
    const auto halfTolerance = std::chrono::nanoseconds(kDefaultPlannerToleranceNs / 2);
    std::this_thread::sleep_for(halfTolerance);
    ASSERT_TRUE(commandValidate(envelope, errorCode));

    /*
     *Invoke a huge delay - this must fail
     */
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_FALSE(commandValidate(envelope, errorCode));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
