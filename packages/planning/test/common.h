#include "packages/estimation/estimator.h"
#include "packages/perception/perception.h"
#include "packages/planning/planner.h"
#include "packages/planning/proto/trajectory_planner_options.pb.h"
#include "packages/planning/utils.h"

namespace planning {
namespace test {

    class DefaultTrajectoryPlanner : public TrajectoryPlanner {
    public:
        DefaultTrajectoryPlanner()
            : TrajectoryPlanner(TrajectoryPlannerOptions()) {
            m_state = PlannerState::PLANNER_INACTIVE;
        }

        bool planTo(__attribute__((unused)) const Sophus::SE3d& target, Trajectory& trajectory) override {
            // Traverse
            trajectory.add_elements();
            // Stop
            trajectory.add_elements();
            return true;
        }
    };

    class TimedDummyPlanner : public planning::TrajectoryPlanner {
    public:
        TimedDummyPlanner(const planning::TrajectoryPlannerOptions& options, float time_in_seconds)
            : planning::TrajectoryPlanner(options)
            , m_time_in_seconds(time_in_seconds) {
            CHECK(m_time_in_seconds > 0);
        }

        bool planTo(__attribute__((unused)) const Sophus::SE3d& target, planning::Trajectory& trajectory) {
            trajectory.clear_elements();
            core::Duration relative_duration = planning::toDuration(m_time_in_seconds);
            auto traverse = trajectory.add_elements();
            traverse->set_curvature(0.0);
            traverse->set_linear_velocity(0.0);
            *traverse->mutable_relative_time() = relative_duration;
            auto stop = trajectory.add_elements();
            stop->set_curvature(0.0);
            stop->set_linear_velocity(0.0);
            return true;
        }

    private:
        const float m_time_in_seconds;
    };

    // VCU does first order, instead of zero-order interpolation
    constexpr int kNumSquareTraverseSegments = 2;
    constexpr int kNumSquareStopSegments = 1;
    constexpr int kExpectedNumberSegments = kNumSquareTraverseSegments + kNumSquareStopSegments;

    template <int ORDER> class TrajectoryVelocityInterpolator {
    public:
        void interpolate(__attribute__((unused)) const Trajectory& trajectory) {}
    };

    template <> class TrajectoryVelocityInterpolator<0> {
    public:
        void interpolate(const Trajectory& trajectory) {
            CHECK(trajectory.elements_size() > 1);
            auto local = trajectory;
            std::vector<float> velocities;
            std::vector<float> times;
            for (const auto& element : *local.mutable_elements()) {
                velocities.push_back(element.linear_velocity());
                times.push_back(element.relative_time().nanos());
            }
            std::vector<float> absoluteTimes;
            std::partial_sum(times.begin(), times.end(), std::back_inserter(absoluteTimes));
            std::for_each(absoluteTimes.begin(), absoluteTimes.end(), [](float& time) { time /= 1e9f; });
        }
    };

    class DefaultPathPlanner : public PathPlanner {
    public:
        DefaultPathPlanner() {}
        bool planTo(__attribute__((unused)) const Sophus::SE3d& point, __attribute__((unused)) Path& path) override { return true; }
    };

    class MockComms : public Comms {
    public:
        bool send(__attribute__((unused)) const Trajectory& trajectory,
            __attribute__((unused)) hal::VCUCommandResponse* optional_response = nullptr) override {
            return true;
        }
    };

    class DefaultPlannerStateMachine : public TrajectoryPlanner {
    public:
        DefaultPlannerStateMachine(std::shared_ptr<TrajectoryPlanner> planner)
            : TrajectoryPlanner(planner->options())
            , m_planner(planner) {
            m_state = PlannerState::PLANNER_INACTIVE;
        }

        bool planTo(const Sophus::SE3d& target, Trajectory& trajectory) override {
            m_state = PlannerState::PLANNER_ACTIVE;
            bool success = m_planner->planTo(target, trajectory);
            m_state = PlannerState::PLANNER_INACTIVE;
            return success;
        }

    private:
        std::shared_ptr<TrajectoryPlanner> m_planner;
        class StateMachineAdapterImpl;
        std::shared_ptr<StateMachineAdapterImpl> m_impl;
    };

    // Test class to simulate a long-running planning query
    class DelayedPlanner : public TrajectoryPlanner {
    public:
        DelayedPlanner()
            : TrajectoryPlanner(TrajectoryPlannerOptions())
            , m_nextReturnValue(true)
            , m_delayMilliseconds(0) {}

        bool planTo(__attribute__((unused)) const Sophus::SE3d& target, __attribute__((unused)) Trajectory& trajectory) override {
            if (m_delayMilliseconds > 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_delayMilliseconds));
            }
            return m_nextReturnValue;
        }

        void setNextReturnValue(bool returnValue) { m_nextReturnValue = returnValue; }

        void setDelay(int milliseconds) {
            CHECK(milliseconds > 0);
            m_delayMilliseconds = milliseconds;
        }

    private:
        bool m_nextReturnValue;
        int m_delayMilliseconds;
    };

} // test
} // planning

namespace estimation {
namespace test {

    class DefaultEstimator : public Estimator {
    public:
        DefaultEstimator() {}
    };

} // test
} // estimation

namespace perception {
namespace test {
    class DefaultPerception : public Perception {
    public:
        DefaultPerception(const PerceptionOptions& options)
            : Perception(options, std::unique_ptr<TerrainRepresentation>()) {}

        bool imageToPoint(__attribute__((unused)) const std::string& camera,
            __attribute__((unused)) const teleop::PointAndGoCommand& command, __attribute__((unused)) core::Point3d& target) override {
            return true;
        }
    };

    class SafetyPerception : public Perception {
    public:
        SafetyPerception(const PerceptionOptions& options)
            : Perception(options, std::unique_ptr<TerrainRepresentation>()) {}
    };

} // test
} // perception
