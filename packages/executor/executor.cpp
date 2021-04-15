#include "packages/executor/executor.h"

#include <chrono>
#include <thread>

#include "packages/core/include/chrono.h"
#include "packages/planning/utils.h"

#include "glog/logging.h"

namespace executor {

using core::chrono::gps::wallClockInNanoseconds;

using estimation::Estimator;
using planning::elementsToPose;
using planning::Comms;
using planning::PlannerState;
using planning::Trajectory;
using planning::TrajectoryPlanner;

Response validateTimestamp(const teleop::PointAndGoCommand& command, const std::chrono::nanoseconds& window) {
    auto currentSystemTime = wallClockInNanoseconds();
    auto commandTime = std::chrono::nanoseconds(command.operatortimestamp().nanos());
    ExecutorResponse response;
    response.set_code(ExecutorResponse::VALID);
    // Check that the command is not in the future
    if ((commandTime - currentSystemTime) > window) {
        LOG(ERROR) << "Command outside of window (future)!";
        LOG(ERROR) << "Now: " << currentSystemTime.count();
        LOG(ERROR) << "Message: " << command.DebugString();
        LOG(ERROR) << "Difference (ms)" << std::chrono::duration_cast<std::chrono::milliseconds>(currentSystemTime - commandTime).count();
        response.set_code(ExecutorResponse::COMMAND_TOO_FAR_IN_FUTURE);
        return Response(response);
    }
    // Check that the command is within some realistic epsilon of now
    if ((currentSystemTime - commandTime) > window) {
        LOG(ERROR) << "Command outside of window (past)!";
        LOG(ERROR) << "Now: " << currentSystemTime.count();
        LOG(ERROR) << "Message: " << command.DebugString();
        LOG(ERROR) << "Window (ms): " << window.count();
        LOG(ERROR) << "Difference (ms)" << std::chrono::duration_cast<std::chrono::milliseconds>(currentSystemTime - commandTime).count();
        response.set_code(ExecutorResponse::COMMAND_TOO_FAR_IN_PAST);
        return Response(response);
    }
    return response;
}

Executor::Executor(const ExecutorOptions& options, std::unique_ptr<Estimator> estimator, std::unique_ptr<TrajectoryPlanner> planner,
    std::unique_ptr<perception::Perception> perception, std::unique_ptr<Comms> comms)
    : m_options(options)
    , m_shutdown(false)
    , m_vehicleState(VehicleState::VEHICLE_UNKNOWN)
    , m_executorState(ExecutorState::EXECUTOR_INACTIVE)
    , m_estimator(std::move(estimator))
    , m_planner(std::move(planner))
    , m_perception(std::move(perception))
    , m_comms(std::move(comms)) {
    if (!options.logfile().empty()) {
        LOG(INFO) << "Initialising logging @: " << options.logfile();
        m_logger.reset(new Logger(options.logfile()));
        CHECK_NOTNULL(m_logger.get());
    }
    CHECK_NOTNULL(m_estimator.get());
    CHECK_NOTNULL(m_planner.get());
    CHECK_NOTNULL(m_perception.get());
    CHECK_NOTNULL(m_comms.get());
    // By default, assume we have known initial state. TODO: This needs to be
    // populated by communicating with the VCU (AUTO-455)
    setVehicleState(VehicleState::VEHICLE_STOPPED);

    if (m_logger.get() != nullptr) {
        m_logger->log(VehicleState::VEHICLE_UNKNOWN, m_vehicleState);
    }
}

Executor::~Executor() {
    if (m_execution_thread.joinable()) {
        m_execution_thread.join();
    }
    m_shutdown = true;
}

void Executor::setVehicleState(const VehicleState& new_state) {
    std::lock_guard<std::mutex> lock(m_vehicleStateGuard);
    m_vehicleState = new_state;
}

void Executor::setExecutorState(const ExecutorState& new_state) {
    std::lock_guard<std::mutex> lock(m_executorStateGuard);
    m_executorState = new_state;
}

Response Executor::execute(const teleop::PointAndGoCommand& command) {
    CHECK(command.imagex() >= 0 and command.imagex() <= 1);
    CHECK(command.imagey() >= 0 and command.imagey() <= 1);

    std::chrono::milliseconds tolerance(m_options.offsettolerancems());

    // Check timestamp
    auto result = validateTimestamp(command, std::chrono::duration_cast<std::chrono::nanoseconds>(tolerance));
    if (!result) {
        return result;
    }
    core::Point3d target;
    if (!m_perception->imageToPoint(command.camera(), command, target)) {
        LOG(ERROR) << "Could not intersect: " << command.DebugString();
        ExecutorResponse response;
        response.set_code(ExecutorResponse::PERCEPTION_REJECTED);
        return Response(response);
    }
    return execute(target);
}

Response Executor::execute(__attribute__((unused)) const teleop::JoystickCommand& command) {
    LOG(ERROR) << "Currently unsupported!";
    ExecutorResponse response;
    response.set_code(ExecutorResponse::UNSUPPORTED);
    return Response(response);
}

Response Executor::execute(const teleop::StopCommand& command) {
    LOG(INFO) << "Received stop: " << command.DebugString();
    Trajectory stopTrajectory;
    constexpr float kDefaultDurationEpsilon = 0.2; // Seconds
    core::Duration stopDuration = planning::toDuration(kDefaultDurationEpsilon);
    auto pre = stopTrajectory.add_elements();
    *pre->mutable_relative_time() = stopDuration;
    auto post = stopTrajectory.add_elements();
    *post->mutable_relative_time() = stopDuration;
    LOG(INFO) << stopTrajectory.DebugString();
    return execute(stopTrajectory);
}

bool Executor::wait() {
    if (!m_execution_thread.joinable()) {
        LOG(ERROR) << "Not currently executing!";
        return false;
    }
    m_execution_thread.join();
    return true;
}

/// TODO: AUTO-440 : resolve SRP violation
bool Executor::planTrajectory(const core::Point3d& target, Trajectory& trajectory) {
    switch (vehicleState()) {
    case VehicleState::VEHICLE_UNKNOWN:
        LOG(FATAL) << "Zippy in unknown state, cannot continue";
        exit(EXIT_FAILURE);
    default:
        // We are ready to execute;
        break;
    }
    Sophus::SE3d target_pose;
    elementsToPose(target.x(), target.y(), 0.0, 0.0, 0.0, 0.0, target_pose);
    if (!m_planner->planTo(target_pose, trajectory)) {
        LOG(ERROR) << "Failed to plan to target";
        return false;
    };
    return true;
}

Response Executor::execute(const core::Point3d& target) {
    Trajectory trajectory;
    LOG(INFO) << "Received: " << target.DebugString();
    if (!planTrajectory(target, trajectory)) {
        ExecutorResponse response;
        response.set_code(ExecutorResponse::PLANNER_REJECTED);
        return response;
    }
    CHECK(trajectory.elements_size() != 0);
    return execute(trajectory);
}

bool Executor::stop() {
    setVehicleState(VehicleState::VEHICLE_STOPPED);
    setExecutorState(ExecutorState::EXECUTOR_INACTIVE);
    return true;
}

bool Executor::interrupt() {
    CHECK(executorState() == ExecutorState::EXECUTOR_ACTIVE);
    CHECK(m_execution_thread.joinable());
    m_continuePlanning = false;
    return wait();
}

Response Executor::execute(const Trajectory& trajectory) {
    switch (executorState()) {
    case ExecutorState::EXECUTOR_ACTIVE:
        LOG(INFO) << "Interrupting previous command!";
        // Interrupt
        CHECK(interrupt());
        break;
    default:
        // Ready to plan
        break;
    }

    CHECK(m_options.policy() == ExecutionPolicy::TIME) << ExecutionPolicy_Name(m_options.policy());
    if (m_execution_thread.joinable()) {
        m_execution_thread.join();
    }
    setExecutorState(ExecutorState::EXECUTOR_ACTIVE);
    m_execution_thread = std::thread([this, trajectory] { return this->executeTimePolicy(trajectory); });

    ExecutorResponse response;
    response.set_code(ExecutorResponse::VALID);
    return Response(response);
}

Response Executor::executeTimePolicy(const Trajectory& trajectory) {
    CHECK(trajectory.elements_size() > 0);
    CHECK(m_planner->state() == PlannerState::PLANNER_INACTIVE) << PlannerState_Name(m_planner->state());
    m_continuePlanning = true;

    auto delayInMs = m_options.plannerdelayms();
    CHECK(delayInMs > 0);
    auto delayInNs = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(delayInMs));

    Trajectory absolute_trajectory;
    relativeTrajectoryToAbsolute(trajectory, delayInNs, absolute_trajectory);
    CHECK(m_comms->send(absolute_trajectory)) << "Failed send/receive acknowledgement";

    // We might have been interrupted, and so still moving. Should get this from
    // the VCU.
    if (vehicleState() != VehicleState::VEHICLE_MOVING) {
        setVehicleState(VehicleState::VEHICLE_MOVING);
    }
    CHECK(absolute_trajectory.elements_size() >= 2) << absolute_trajectory.DebugString();
    auto end = std::chrono::nanoseconds(absolute_trajectory.elements(absolute_trajectory.elements_size() - 1).absolute_time().nanos());
    CHECK(end.count() > 0);

    ExecutorResponse response;
    response.set_code(ExecutorResponse::VALID);

    while (core::chrono::gps::wallClockInNanoseconds() <= end) {
        auto countdown = (end - core::chrono::gps::wallClockInNanoseconds());
        LOG(INFO) << std::chrono::duration_cast<std::chrono::milliseconds>(countdown).count();
        if (!m_continuePlanning) {
            LOG(INFO) << "interrupt received";
            return Response(response);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    stop();
    return Response(response);
}

} // executor
