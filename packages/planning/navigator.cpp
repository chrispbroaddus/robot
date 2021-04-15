#include "packages/planning/navigator.h"

#include "glog/logging.h"

namespace planning {

DefaultPointAndGoNavigator::DefaultPointAndGoNavigator(
    std::unique_ptr<Comms> vcuComms, std::unique_ptr<TrajectoryPlanner> trajectoryPlanner)
    : Navigator(NavigatorOptions(), std::move(vcuComms), std::move(trajectoryPlanner), std::unique_ptr<PathPlanner>(),
          std::shared_ptr<estimation::Estimator>()) {}

std::future<bool> DefaultPointAndGoNavigator::navigateTo(const Sophus::SE3d& target) {
    // There is no path planner associated
    Trajectory trajectory;
    CHECK(m_trajectoryPlanner.get() != nullptr);
    if (!m_trajectoryPlanner->planTo(target, trajectory)) {
        LOG(ERROR) << " Failed to plan to target";
        m_promise.set_value(false);
    }
    CHECK(trajectory.elements_size() > 0);
    m_promise.set_value(true);
    return m_promise.get_future();
}

bool EuclideanDistanceDecisionFunction::operator()(const Sophus::SE3d& current, const Sophus::SE3d& goal) const {
    return ((current.translation() - goal.translation()).norm() < m_tolerance);
}

bool FallbackPlanner::planAcross(const Path& path, TrajectoryPlanner& planner, Trajectory& trajectory) {
    CHECK(path.elements_size() > 0);
    trajectory.clear_elements();
    Sophus::SE3d previous(Eigen::Quaterniond{ 1, 0, 0, 0 }, Eigen::Vector3d{ 0, 0, 0 });
    auto local_path = path;
    for (const auto& next : *local_path.mutable_elements()) {
        Sophus::SE3d next_;
        protoToPose(next.transform(), next_);
        auto relative = previous.inverse() * next_;
        CHECK(relative.translation().norm() > 0) << path.DebugString();
        if (!planner.planTo(relative, trajectory)) {
            return false;
        }
        previous = next_;
    }
    return true;
}

WaypointNavigator::~WaypointNavigator() { stop(); }

void WaypointNavigator::stop() {
    m_active = false;
    if (m_executionThread.joinable()) {
        m_executionThread.join();
    }
}

bool WaypointNavigator::navigateToWaypoint(const Path& path, int waypointCounter, const Sophus::SE3d& baseFrame) {
    CHECK(waypointCounter >= 0);
    CHECK(waypointCounter < path.elements_size());

    Path remainingWaypoints;
    for (int i = waypointCounter; i < path.elements_size(); ++i) {
        auto* waypoint = remainingWaypoints.add_elements();
        *waypoint = path.elements(i);
    }

    const int plannerSleepDelay = static_cast<int>(1 / m_options.frequency() * 1000);

    const PathElement& waypoint = path.elements(waypointCounter);

    std::for_each(
        m_waypointCallbacks.begin(), m_waypointCallbacks.end(), [waypoint, baseFrame](WaypointCallback& cb) { cb(waypoint, baseFrame); });

    Sophus::SE3d goal;
    protoToPose(waypoint.transform(), goal);

    while (true) {
        // Update pose
        auto state = m_estimator->states().front();

        // Resolve pose into vehicle frame
        auto relativePose = baseFrame.inverse() * state.m_pose;

        // At goal?
        if (m_decisionFunction->operator()(relativePose, goal)) {
            return true;
        }

        estimation::State relativeState;
        relativeState.m_pose = relativePose;

        Path remainingWaypoints;
        for (int i = waypointCounter; i < path.elements_size(); ++i) {
            auto* waypoint = remainingWaypoints.add_elements();
            *waypoint = path.elements(i);
        }

        transform(relativePose.inverse(), remainingWaypoints);
        Trajectory completeTrajectory;
        CHECK(m_fallbackPlanner.planAcross(remainingWaypoints, *m_trajectoryPlanner, completeTrajectory));

        std::for_each(m_fallbackTrajectoryCallbacks.begin(), m_fallbackTrajectoryCallbacks.end(),
            [completeTrajectory, relativeState, remainingWaypoints](
                          FallbackTrajectoryCallback& cb) { cb(completeTrajectory, relativeState, remainingWaypoints); });

        // Send command to VCU
        Trajectory absoluteTrajectory;
        relativeTrajectoryToAbsolute(completeTrajectory, m_planningTolerance, absoluteTrajectory);
        CHECK(m_vcuComms->send(absoluteTrajectory));
        // Log
        m_trajectoryLogger->add(absoluteTrajectory);

        std::for_each(m_trajectoryCallbacks.begin(), m_trajectoryCallbacks.end(),
            [absoluteTrajectory, relativeState](TrajectoryCallback& cb) { cb(absoluteTrajectory, relativeState); });

        std::this_thread::sleep_for(std::chrono::milliseconds(plannerSleepDelay));
    }
    return false;
}

void WaypointNavigator::execute(const Sophus::SE3d& target) {
    auto states = m_estimator->states();
    while (states.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        states = m_estimator->states();
    }
    CHECK(!states.empty());
    const auto baseFrame = m_estimator->states().front();

    Path path;
    if (!m_pathPlanner->planTo(target, path)) {
        LOG(ERROR) << "Failed to generate path plan for :" << target.translation();
        return;
    }

    std::for_each(m_pathCallbacks.begin(), m_pathCallbacks.end(), [path, baseFrame](PathCallback& cb) { cb(path, baseFrame); });

    auto subsampleWaypoints = [](const Path& path) -> Path {
        Path sampled_path;
        constexpr double kWaypointDistance = 0.5; // (m)
        double distance = 0; // (m)

        Eigen::Vector3d previous = Eigen::Vector3d::Zero();

        for (int i = 0; i < path.elements_size(); ++i) {
            const auto element = path.elements(i);
            Sophus::SE3d se3_element;
            protoToPose(element.transform(), se3_element);
            Eigen::Vector3d current = se3_element.translation();
            distance += (current - previous).norm();
            if (distance > kWaypointDistance) {
                auto* sampled_element = sampled_path.add_elements();
                *sampled_element = element;
                distance = 0;
            }
            previous = current;
        }
        auto* goal = sampled_path.add_elements();
        *goal = path.elements(path.elements_size() - 1);
        return sampled_path;
    };

    auto waypointPath = subsampleWaypoints(path);

    int waypointCounter = 0;

    bool atGoal = false;

    while (true) {
        if (!m_active) {
            LOG(ERROR) << "Interrupted!";
            break;
        }

        if (!navigateToWaypoint(waypointPath, waypointCounter, baseFrame.m_pose)) {
            LOG(ERROR) << "Waypoint execution failed!";
            break;
        }

        if (++waypointCounter == waypointPath.elements_size()) {
            Trajectory trajectory, absoluteTrajectory;
            core::Duration stop_time;
            stop_time.set_nanos(static_cast<int>(1e6));
            auto stop = trajectory.add_elements();
            *stop->mutable_relative_time() = stop_time;
            relativeTrajectoryToAbsolute(trajectory, m_planningTolerance, absoluteTrajectory);
            CHECK(m_vcuComms->send(absoluteTrajectory));
            LOG(INFO) << "Execution finished";
            atGoal = true;
            break;
        }
    }
    m_active = false;
    m_promise.set_value(atGoal);
}

std::future<bool> WaypointNavigator::navigateTo(const Sophus::SE3d& target) {
    stop();
    CHECK(!m_active);
    m_active = true;
    m_promise = std::promise<bool>();
    m_executionThread = std::thread(&WaypointNavigator::execute, this, target);
    return m_promise.get_future();
}

void WaypointNavigator::addWaypointCallback(WaypointCallback callback) { m_waypointCallbacks.emplace_back(callback); }

void WaypointNavigator::addPathCallback(PathCallback callback) { m_pathCallbacks.emplace_back(callback); }

void WaypointNavigator::addTrajectoryCallback(TrajectoryCallback callback) { m_trajectoryCallbacks.emplace_back(callback); }

void WaypointNavigator::addFallbackTrajectoryCallback(FallbackTrajectoryCallback callback) {
    m_fallbackTrajectoryCallbacks.emplace_back(callback);
}

} // planning
