#include "packages/executor/logging.h"

namespace executor {

using planning::getTime;
using planning::Trajectory;
using planning::PlannerState;

void Logger::log(const core::Point3d& point_and_go_request) {
    executor::LogEntry entry;
    *entry.mutable_time() = getTime();
    *entry.mutable_point_and_go_request() = point_and_go_request;
    CHECK(add(entry));
}

void Logger::log(const Trajectory& planned_trajectory) {
    executor::LogEntry entry;
    *entry.mutable_time() = getTime();
    *entry.mutable_planned_trajectory() = planned_trajectory;
    CHECK(add(entry));
}

void Logger::log(const PlannerStateChange& change) {
    executor::LogEntry entry;
    *entry.mutable_time() = getTime();
    *entry.mutable_planner_state_change() = change;
    std::lock_guard<std::mutex> guard(m_lock);
    m_queue.emplace_back(entry);
}

void Logger::log(const VehicleStateChange& change) {
    executor::LogEntry entry;
    *entry.mutable_time() = getTime();
    *entry.mutable_vehicle_state_change() = change;
    CHECK(add(entry));
}

void Logger::log(const VehicleState& previous_state, const VehicleState& current_state) {
    CHECK(previous_state != current_state);
    VehicleStateChange change;
    change.set_previous(previous_state);
    change.set_current(current_state);
    log(change);
}

void Logger::log(const PlannerState& previous_state, const PlannerState& current_state) {
    CHECK(previous_state != current_state);
    PlannerStateChange change;
    change.set_previous(previous_state);
    change.set_current(current_state);
    log(change);
}

} // executor
