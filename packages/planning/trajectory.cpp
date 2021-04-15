#include "packages/planning/trajectory.h"
#include "packages/planning/utils.h"

namespace planning {
constexpr char kDefaultTrajectoryOptions[] = "config/global/trajectory_options.default.pbtxt";

TrajectoryOptions loadDefaultTrajectoryOptions() {
    TrajectoryOptions options;
    CHECK(loadOptions(kDefaultTrajectoryOptions, options));
    return options;
}

bool validateTrajectoryOptions(const TrajectoryOptions& options) {
    if (options.frequency() <= 0) {
        return false;
    }
    CHECK(options.max_velocity() > 0);
    if (options.max_velocity() > 2) {
        return false;
    }
    CHECK(options.max_acceleration() > 0);
    if (options.max_acceleration() > 5) {
        return false;
    }
    CHECK(options.max_deceleration() > 0);
    if (options.max_deceleration() > 5) {
        return false;
    }
    return true;
}

void addDecelerationSegment(Trajectory& trajectory, const TrajectoryOptions& options) {
    CHECK(validateTrajectoryOptions(options));
    CHECK(trajectory.elements_size() > 0);
    auto last_element = trajectory.elements(trajectory.elements_size() - 1);
    CHECK(last_element.linear_velocity() == options.max_velocity()) << trajectory.DebugString();

    const double t_d = options.max_velocity() / options.max_deceleration();
    const double dT = 1 / options.frequency();
    const double maxDeceleration = options.max_deceleration();

    double v_0 = options.max_velocity();
    double v_t = v_0;
    for (double t = dT; t <= t_d; t += dT) {
        v_t = (v_0 - maxDeceleration * t);
        auto decelerate = trajectory.add_elements();
        decelerate->set_curvature(last_element.curvature());
        decelerate->set_linear_velocity(v_t);
        core::Duration relativeDuration = toDuration(t);
        *decelerate->mutable_relative_time() = relativeDuration;
    }
}

bool SquareTrajectory::generate(double arc_length, double curvature, Trajectory& trajectory) {
    const double velocity = m_options.max_velocity();
    const double duration = arc_length / velocity;

    CHECK(duration > 0) << "Computed negative duration";
    trajectory.set_type(TrajectoryType::SQUARE);

    core::Duration relativeDuration = toDuration(duration);
    auto traverseBegin = trajectory.add_elements();
    traverseBegin->set_curvature(curvature);
    traverseBegin->set_linear_velocity(velocity);
    *traverseBegin->mutable_relative_time() = relativeDuration;
    traverseBegin->set_arclength(arc_length);

    auto traverseEnd = trajectory.add_elements();
    constexpr double kDefaultDurationEpsilon = 0.1; // Seconds
    core::Duration epsilonDuration = toDuration(kDefaultDurationEpsilon);
    traverseEnd->set_curvature(curvature);
    traverseEnd->set_linear_velocity(velocity);
    *traverseEnd->mutable_relative_time() = epsilonDuration;
    traverseEnd->set_arclength(0.0);

    auto stop = trajectory.add_elements();
    stop->set_linear_velocity(0.0);
    stop->set_curvature(curvature);
    return true;
}

bool SquareTrajectoryWithDeceleration::generate(double arc_length, double curvature, Trajectory& trajectory) {
    CHECK(SquareTrajectory::generate(arc_length, curvature, trajectory));
    Trajectory decelerationTrajectory = trajectory;
    // Remove the stop
    decelerationTrajectory.mutable_elements()->RemoveLast();
    // Add deceleration
    addDecelerationSegment(decelerationTrajectory, m_options);
    auto lastElement = decelerationTrajectory.elements(decelerationTrajectory.elements_size() - 1);
    auto stop = decelerationTrajectory.add_elements();
    stop->set_curvature(lastElement.curvature());
    trajectory = decelerationTrajectory;
    return true;
}

class TrapezoidalTrajectory::TrapezoidalTrajectoryImpl {
public:
    explicit TrapezoidalTrajectoryImpl(TrajectoryOptions& options)
        : m_maxVelocity(options.max_velocity())
        , m_maxAcceleration(options.max_acceleration())
        , m_maxDeceleration(options.max_deceleration())
        , m_frequency(options.frequency()) {
        CHECK(options.max_velocity() > 0);
        CHECK(options.max_acceleration() > 0);
        CHECK(options.max_deceleration() > 0);
        CHECK(options.frequency() > 0);
    }

    bool generate(double arc_length, double curvature, Trajectory& trajectory) {
        CHECK(arc_length > 0);
        // Is this a triangular trajectory?
        const double timeToMaxVelocity = m_maxVelocity / m_maxAcceleration;
        const double distanceToMaxVelocity = m_maxAcceleration * timeToMaxVelocity * timeToMaxVelocity / 2;
        if (2 * distanceToMaxVelocity >= arc_length) {
            trajectory.set_type(TrajectoryType::TRIANGULAR);
            return generateTriangularTrajectory(arc_length, curvature, trajectory);
        } else {
            trajectory.set_type(TrajectoryType::TRAPEZOIDAL);
            return generateTrapezoidalTrajectory(arc_length, curvature, trajectory);
        }
    }

    bool generateTriangularTrajectory(double arc_length, double curvature, Trajectory& trajectory) {
        // 1. Compute midpoint
        double t_midpoint = std::sqrt(arc_length / m_maxAcceleration);
        CHECK(t_midpoint > 0);
        // 2. Setpoints at 1/frequency: acceleration
        double v_0 = 0.0;
        double t_start = 0;
        double v_t = v_0;

        const double dT = 1 / m_frequency;

        for (double t = t_start; t < t_midpoint; t += dT) {
            v_t = (v_0 + m_maxAcceleration * t);
            auto accelerate = trajectory.add_elements();
            accelerate->set_curvature(curvature);
            accelerate->set_linear_velocity(v_t);
            core::Duration relativeDuration = toDuration(t_start + t);
            *accelerate->mutable_relative_time() = relativeDuration;
        }
        auto v_midpoint = v_t;
        // 3. Setpoints at 1/frequency: deceleration
        for (double t = dT; t < t_midpoint; t += dT) {
            v_t = v_midpoint - m_maxDeceleration * t;
            auto decelerate = trajectory.add_elements();
            decelerate->set_curvature(curvature);
            decelerate->set_linear_velocity(v_t);
            core::Duration relativeDuration = toDuration(t_midpoint + t);
            *decelerate->mutable_relative_time() = relativeDuration;
        }
        return true;
    }

    bool generateTrapezoidalTrajectory(double arc_length, double curvature, Trajectory& trajectory) {
        // 1. Compute transition points
        auto t_a = m_maxVelocity / m_maxAcceleration;
        CHECK(t_a > 0);
        auto d_a = m_maxAcceleration * (t_a * t_a * 2);
        auto d_d = d_a;
        CHECK((d_d + d_a) < arc_length);
        auto d_trav = arc_length - (d_d + d_a);
        CHECK(d_trav > 0);
        auto t_trav = d_trav / m_maxVelocity;
        auto t_finish = 2 * t_a + t_trav;
        auto t_d = t_finish - t_a;
        CHECK(t_d > 0);
        CHECK(t_d > t_a);

        const double dT = 1 / m_frequency;

        // 2. Setpoints at 1/frequency for acc
        double v_0 = 0.0;
        double v_t = v_0;
        for (double t = 0; t <= t_a; t += dT) {
            v_t = (v_0 + m_maxAcceleration * t);
            auto accelerate = trajectory.add_elements();
            accelerate->set_curvature(curvature);
            accelerate->set_linear_velocity(v_t);
            core::Duration relativeDuration = toDuration(t);
            *accelerate->mutable_relative_time() = relativeDuration;
        }
        // 3. Constant setpoint for traverse
        CHECK(v_t == m_maxVelocity) << v_t;
        {
            auto traverse = trajectory.add_elements();
            traverse->set_curvature(curvature);
            traverse->set_linear_velocity(m_maxVelocity);
            core::Duration relativeDuration = toDuration(t_a + .01); /// BRD: this looks very suspicious
            *traverse->mutable_relative_time() = relativeDuration;
        }
        // 4. Setpoints at 1/frequency for dec
        for (double t = 0; t <= t_a; t += dT) {
            v_t = (m_maxVelocity - 1 * m_maxDeceleration * t);
            auto decelerate = trajectory.add_elements();
            decelerate->set_curvature(curvature);
            decelerate->set_linear_velocity(v_t);
            core::Duration relativeDuration = toDuration(t_d + t);
            *decelerate->mutable_relative_time() = relativeDuration;
        }
        CHECK(v_t == 0) << v_t;
        return true;
    }

private:
    const double m_maxVelocity;
    const double m_maxAcceleration;
    const double m_maxDeceleration;
    const double m_frequency;
};

TrapezoidalTrajectory::TrapezoidalTrajectory(TrajectoryOptions& options)
    : TrajectoryGenerator(options)
    , m_impl(new TrapezoidalTrajectoryImpl(options)) {
    CHECK(m_options.max_velocity() > 0);
    CHECK(m_options.max_acceleration() > 0);
}

bool TrapezoidalTrajectory::generate(double arc_length, double curvature, Trajectory& trajectory) {
    CHECK_NOTNULL(m_impl.get());
    return m_impl->generate(arc_length, curvature, trajectory);
}

} // planning
