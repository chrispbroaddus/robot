#pragma once
#include <future>
#include <memory>

#include "packages/estimation/estimator.h"
#include "packages/hal/proto/vcu_trajectory_command.pb.h"
#include "packages/planning/logging.h"
#include "packages/planning/planner.h"
#include "packages/planning/proto/navigator_options.pb.h"
#include "packages/planning/utils.h"

namespace planning {

/**
 * @brief The navigation interface. A navigator is responsible for moving the
 * vehicle to a specified target using the given path and trajectory planners
 */
class Navigator {
public:
    Navigator(const NavigatorOptions& options, std::unique_ptr<Comms> vcuComms, std::unique_ptr<TrajectoryPlanner> trajectoryPlanner,
        std::unique_ptr<PathPlanner> pathPlanner, std::shared_ptr<estimation::Estimator> estimator)
        : m_options(options)
        , m_vcuComms(std::move(vcuComms))
        , m_pathPlanner(std::move(pathPlanner))
        , m_trajectoryPlanner(std::move(trajectoryPlanner))
        , m_estimator(estimator) {}

    virtual std::future<bool> navigateTo(const Sophus::SE3d& pose) = 0;

    virtual void stop() = 0;

    virtual ~Navigator() = default;

protected:
    const NavigatorOptions m_options;
    std::unique_ptr<Comms> m_vcuComms;
    std::unique_ptr<PathPlanner> m_pathPlanner;
    std::unique_ptr<TrajectoryPlanner> m_trajectoryPlanner;
    std::shared_ptr<estimation::Estimator> m_estimator;
};

/**
 * @brief The default point-and-go planner. This has no knowledge of
 * path-planning, it is purely a reactive planner.
 */
class DefaultPointAndGoNavigator : public Navigator {
public:
    DefaultPointAndGoNavigator(std::unique_ptr<Comms> vcuComms, std::unique_ptr<TrajectoryPlanner> trajectoryPlanner);

    std::future<bool> navigateTo(const Sophus::SE3d& target) override;

    void stop() override{};

private:
    std::promise<bool> m_promise;
};

/**
 * @brief a utility class to decide whether the robot has achieved a given goal
 */
class GoalDecisionFunction {
public:
    virtual bool operator()(const Sophus::SE3d& current, const Sophus::SE3d& goal) const = 0;

    virtual ~GoalDecisionFunction() = default;
};

/**
 * @brief The simplest possible implementation of a decision function - checks
 * whether the vehicle is within some pre-defined tolerance of the given goal
 */
class EuclideanDistanceDecisionFunction : public GoalDecisionFunction {
public:
    EuclideanDistanceDecisionFunction(double tolerance = 0.1)
        : m_tolerance(tolerance) {
        CHECK(m_tolerance > 0);
    }

    bool operator()(const Sophus::SE3d& current, const Sophus::SE3d& goal) const override;

private:
    const double m_tolerance;
};

class FallbackPlanner {
public:
    bool planAcross(const Path& path, TrajectoryPlanner& planner, Trajectory& trajectory);
};

/**
 * @brief A waypoint-based navigator; a Path from start to goal is sampled to
 * generate a number of evenly-spaced intermediate waypoints. Each waypoint is
 * navigated to, until the goal is reached
 */
class WaypointNavigator : public Navigator {
public:
    WaypointNavigator(const NavigatorOptions& options, std::unique_ptr<Comms> vcuComms,
        std::unique_ptr<TrajectoryPlanner> trajectoryPlanner, std::unique_ptr<PathPlanner> pathPlanner,
        std::shared_ptr<estimation::Estimator> estimator, std::unique_ptr<GoalDecisionFunction> decisionFunction)
        : Navigator(options, std::move(vcuComms), std::move(trajectoryPlanner), std::move(pathPlanner), estimator)
        , m_decisionFunction(std::move(decisionFunction))
        , m_active(false) {
        m_trajectoryLogger.reset(new BaseLogger<Trajectory>("navigator.log"));
    }

    ~WaypointNavigator();

    std::future<bool> navigateTo(const Sophus::SE3d& target) override;

    void stop() override;

    typedef std::function<void(const PathElement&, const Sophus::SE3d& baseFrame)> WaypointCallback;
    void addWaypointCallback(WaypointCallback callback);

    typedef std::function<void(const Path&, const estimation::State& state)> PathCallback;
    void addPathCallback(PathCallback callback);

    typedef std::function<void(const Trajectory& trajectory, const estimation::State& state)> TrajectoryCallback;
    void addTrajectoryCallback(TrajectoryCallback callback);

    typedef std::function<void(const Trajectory& trajectory, const estimation::State& state, const Path& waypoints)>
        FallbackTrajectoryCallback;
    void addFallbackTrajectoryCallback(FallbackTrajectoryCallback callback);

private:
    WaypointNavigator(const WaypointNavigator&) = delete;
    WaypointNavigator(const WaypointNavigator&&) = delete;
    WaypointNavigator& operator=(const WaypointNavigator&) = delete;
    WaypointNavigator& operator=(const WaypointNavigator&&) = delete;

    bool navigateToWaypoint(const Path& path, int waypointCounter, const Sophus::SE3d& baseFrame);

    void execute(const Sophus::SE3d& target);

    FallbackPlanner m_fallbackPlanner;

    const std::chrono::nanoseconds m_planningTolerance{ static_cast<int>(5e7) };

    std::unique_ptr<GoalDecisionFunction> m_decisionFunction;
    std::thread m_executionThread;
    std::atomic<bool> m_active;
    std::promise<bool> m_promise;

    std::deque<WaypointCallback> m_waypointCallbacks;
    std::deque<PathCallback> m_pathCallbacks;
    std::deque<TrajectoryCallback> m_trajectoryCallbacks;
    std::deque<FallbackTrajectoryCallback> m_fallbackTrajectoryCallbacks;

    std::unique_ptr<BaseLogger<Trajectory> > m_trajectoryLogger;
};

} // planning
