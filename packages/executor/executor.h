#pragma once

#include <mutex>
#include <thread>

#include "packages/core/proto/geometry.pb.h"
#include "packages/core/proto/timestamp.pb.h"
#include "packages/estimation/estimator.h"
#include "packages/executor/logging.h"
#include "packages/executor/proto/executor_options.pb.h"
#include "packages/executor/proto/executor_response.pb.h"
#include "packages/executor/proto/state.pb.h"
#include "packages/hal/proto/vcu_command_response.pb.h"
#include "packages/perception/perception.h"
#include "packages/planning/logging.h"
#include "packages/planning/planner.h"
#include "packages/planning/proto/state.pb.h"
#include "packages/planning/utils.h"
#include "packages/teleop/proto/backend_message.pb.h"

namespace executor {

class Response {
public:
    Response(const ExecutorResponse& response)
        : m_response(response) {}
    operator bool() const { return m_response.code() == ExecutorResponse::VALID; }
    std::string reason() const { return ExecutorResponse_ResponseCode_Name(m_response.code()); }

    const ExecutorResponse& response() const { return m_response; }

private:
    const ExecutorResponse m_response;
};

Response validateTimestamp(const teleop::PointAndGoCommand& command, const std::chrono::nanoseconds& window);

class Executor {
public:
    Executor(const ExecutorOptions& options, std::unique_ptr<estimation::Estimator> estimator,
        std::unique_ptr<planning::TrajectoryPlanner> planner, std::unique_ptr<perception::Perception> perception,
        std::unique_ptr<planning::Comms> comms);

    virtual ~Executor();

    virtual bool planTrajectory(const core::Point3d& target, planning::Trajectory& trajectory);

    virtual Response execute(const core::Point3d& target);

    virtual Response execute(const teleop::PointAndGoCommand& command);

    virtual Response execute(const teleop::JoystickCommand& command);

    virtual Response execute(const teleop::StopCommand& command);

    bool stop();

    bool wait();

    const VehicleState& vehicleState() const {
        std::lock_guard<std::mutex> lock(m_vehicleStateGuard);
        return m_vehicleState;
    }

    const ExecutorState& executorState() const {
        std::lock_guard<std::mutex> lock(m_vehicleStateGuard);
        return m_executorState;
    }

protected:
    Executor(const Executor&) = delete;
    Executor& operator=(const Executor&) = delete;
    Executor& operator=(const Executor&&) = delete;

    Response execute(const planning::Trajectory& trajectory);
    Response executeTimePolicy(const planning::Trajectory& trajectory);
    bool interrupt();
    void setVehicleState(const VehicleState& new_state);
    void setExecutorState(const ExecutorState& new_state);

    const ExecutorOptions& m_options;

    std::atomic_bool m_shutdown;
    std::atomic_bool m_continuePlanning;

    VehicleState m_vehicleState;
    ExecutorState m_executorState;

    std::unique_ptr<estimation::Estimator> m_estimator;
    std::unique_ptr<planning::TrajectoryPlanner> m_planner;
    std::unique_ptr<perception::Perception> m_perception;
    std::unique_ptr<planning::Comms> m_comms;
    std::unique_ptr<Logger> m_logger;
    std::thread m_execution_thread;
    mutable std::mutex m_vehicleStateGuard;
    mutable std::mutex m_executorStateGuard;
};

} // executor
