#pragma once

#include <atomic>
#include <thread>

#include "packages/planning/definitions.h"
#include "packages/planning/proto/path.pb.h"
#include "packages/planning/proto/state.pb.h"
#include "packages/planning/proto/trajectory.pb.h"
#include "packages/planning/proto/trajectory_options.pb.h"
#include "packages/planning/proto/trajectory_planner_options.pb.h"
#include "packages/planning/trajectory.h"
#include "packages/planning/utils.h"

namespace planning {

/**
 * @brief Default trajectory-planner options common across planner types
 *
 * @return TrajectoryPlannerOptions proto
 */
TrajectoryPlannerOptions loadDefaultTrajectoryPlannerOptions();

class TrajectoryPlanner {
public:
    TrajectoryPlanner(const TrajectoryPlannerOptions& options)
        : m_options(options)
        , m_counter(0)
        , m_state(PlannerState::PLANNER_UNINITIALIZED) {}

    virtual bool planTo(const Sophus::SE3d& target, Trajectory& trajectory) = 0;

    virtual ~TrajectoryPlanner() = default;

    const PlannerState& state() const { return m_state; }

    const TrajectoryPlannerOptions& options() const { return m_options; }

private:
    TrajectoryPlanner(const TrajectoryPlanner&) = delete;
    TrajectoryPlanner& operator=(const TrajectoryPlanner&) = delete;
    TrajectoryPlanner& operator=(const TrajectoryPlanner&&) = delete;

protected:
    const TrajectoryPlannerOptions& m_options;
    std::atomic<std::uint64_t> m_counter;
    PlannerState m_state;
};

/**
 * @brief Arc-planner specific planner options
 *
 * @return ArcPlannerOptions proto
 */
ArcPlannerOptions loadDefaultArcPlannerOptions();

/**
 * @brief A planner generating simple arcs of constant curvature
 */
class ArcPlanner : public TrajectoryPlanner {
public:
    ArcPlanner(const ArcPlannerOptions& options, std::unique_ptr<TrajectoryGenerator> generator)
        : TrajectoryPlanner(options.base_options())
        , m_arcOptions(options)
        , m_generator(std::move(generator)) {}

    bool planTo(const Sophus::SE3d& target, Trajectory& trajectory) override;

    const ArcPlannerOptions& options() { return m_arcOptions; }

private:
    const ArcPlannerOptions m_arcOptions;
    std::unique_ptr<TrajectoryGenerator> m_generator;
};

/**
 * @brief An adapter to the ArcPlanner class. This enforces that the ArcPlanner
 *        does not generate trajectories that wont be accepted by the VCU.
 */
class ConstrainedArcWrapper : public ArcPlanner {
public:
    ConstrainedArcWrapper(std::unique_ptr<ArcPlanner> planner)
        : ArcPlanner(planner->options(), std::unique_ptr<TrajectoryGenerator>())
        , m_arcPlanner(std::move(planner)) {}

    bool planTo(const Sophus::SE3d& target, Trajectory& trajectory) override {
        bool success = m_arcPlanner->planTo(target, trajectory);
        if (!success) {
            return false;
        }
        for (auto& element : *trajectory.mutable_elements()) {
            if (element.curvature() > PlannerConstants<float>::kCurvatureLimit) {
                element.set_curvature(PlannerConstants<float>::kCurvatureLimit);
            } else if (element.curvature() < -PlannerConstants<float>::kCurvatureLimit) {
                element.set_curvature(-PlannerConstants<float>::kCurvatureLimit);
            }
        }
        return true;
    };

private:
    std::unique_ptr<ArcPlanner> m_arcPlanner;
};

/**
 * @brief Experimental: This is an adapter to the ArcPlanner class that scales
 *                      the velocity wrt. the curvature. This helps dynamic
 *                      stability.
 */
class ScaledArcWrapper : public ArcPlanner {
public:
    ScaledArcWrapper(std::unique_ptr<ArcPlanner> planner)
        : ArcPlanner(planner->options(), std::unique_ptr<TrajectoryGenerator>())
        , m_arcPlanner(std::move(planner))
        , m_velocityScale(-0.5)
        , m_velocityOffset(1.0) {}

    bool planTo(const Sophus::SE3d& target, Trajectory& trajectory) override {
        bool success = m_arcPlanner->planTo(target, trajectory);
        if (!success) {
            return false;
        }
        for (auto& element : *trajectory.mutable_elements()) {
            const double curvature = element.curvature();
            const double curveRatio = std::abs(curvature) / PlannerConstants<double>::kCurvatureLimit;
            const double velocityScale = (m_velocityScale * curveRatio) + m_velocityOffset;
            element.set_linear_velocity(element.linear_velocity() * static_cast<float>(velocityScale));
        }
        return true;
    };

private:
    std::unique_ptr<ArcPlanner> m_arcPlanner;
    double m_velocityScale;
    double m_velocityOffset;
};

/**
 * @brief Adapts the trajectory planner interface, in order to drive the planner
 * state machine in a principled way.
 */
class StateMachineAdapter : public TrajectoryPlanner {
public:
    StateMachineAdapter(std::shared_ptr<TrajectoryPlanner> planner);

    bool planTo(const Sophus::SE3d& target, Trajectory& trajectory) override;

private:
    std::shared_ptr<TrajectoryPlanner> m_planner;
    class StateMachineAdapterImpl;
    std::shared_ptr<StateMachineAdapterImpl> m_impl;
};

/**
 * @brief The Path planning interfaces. Paths are purely spatial, trajectories
 * spatio-temporal
 */
class PathPlanner {
public:
    virtual bool planTo(const Sophus::SE3d& target, Path& path) = 0;

    virtual ~PathPlanner() = default;
};

/**
 * @brief Interface to the Search-based Planning Library for lattice-based
 * planning
 */
class SBPLPlanner : public PathPlanner {
public:
    SBPLPlanner(const SBPLPlannerOptions& options);

    bool planTo(const Sophus::SE3d& target, Path& path) override;

private:
    class SBPLPlannerImpl;
    std::shared_ptr<SBPLPlannerImpl> m_impl;
};

} // planning
