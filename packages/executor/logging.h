#pragma once

#include "packages/executor/proto/log.pb.h"
#include "packages/executor/proto/state.pb.h"
#include "packages/planning/logging.h"
#include "packages/planning/proto/state.pb.h"

namespace executor {

class Logger : public planning::BaseLogger<LogEntry> {
public:
    Logger(const std::string& log)
        : BaseLogger(log){};

    void log(const PlannerStateChange& planner_state_change);
    void log(const planning::PlannerState& previous_state, const planning::PlannerState& current_state);
    void log(const VehicleStateChange& vehicle_state_change);
    void log(const VehicleState& previous_state, const VehicleState& current_state);
    void log(const core::Point3d& point_and_go_request);
    void log(const planning::Trajectory& planned_trajectory);

private:
    Logger(const Logger& logger) = delete;
    Logger(const Logger&& logger) = delete;
    Logger& operator=(const Logger& logger) = delete;
};
}
