#include "packages/planning/proto/trajectory.pb.h"
#include "packages/planning/proto/trajectory_options.pb.h"

#include "glog/logging.h"

namespace planning {

/**
 * @brief Options specifying the limits for trajectory generation
 *
 * @return TrajectoryOptions proto
 */
TrajectoryOptions loadDefaultTrajectoryOptions();

/**
 * @brief Interface for generating body-frame velocities for Zippy
 */
class TrajectoryGenerator {
public:
    TrajectoryGenerator(TrajectoryOptions& options)
        : m_options(options) {}

    virtual bool generate(double arc_length, double curvature, Trajectory& trajectory) = 0;

    virtual ~TrajectoryGenerator() = default;

    TrajectoryOptions& options() { return m_options; }

protected:
    TrajectoryOptions m_options;
};

class ScaledVelocityTrajectory : public TrajectoryGenerator {
public:
    ScaledVelocityTrajectory(std::unique_ptr<TrajectoryGenerator> generator, const double arcLengthThreshold)
        : TrajectoryGenerator(generator->options())
        , m_generator(std::move(generator))
        , m_arcLengthThreshold(arcLengthThreshold) {
        CHECK(m_arcLengthThreshold > 0);
    }

    bool generate(double arc_length, double curvature, Trajectory& trajectory) override {
        auto& options = m_generator->options();
        auto maxVelocity = options.max_velocity();
        if (arc_length < m_arcLengthThreshold) {
            const double maxVelocity = options.max_velocity();
            auto scaledVelocity = arc_length / m_arcLengthThreshold * maxVelocity;
            scaledVelocity = std::max(scaledVelocity, 0.1);
            options.set_max_velocity(scaledVelocity);
        }
        bool result = m_generator->generate(arc_length, curvature, trajectory);
        options.set_max_velocity(maxVelocity);
        return result;
    }

private:
    std::unique_ptr<TrajectoryGenerator> m_generator;

    const double m_arcLengthThreshold;
};

/**
 * @brief Velocity changes are represented by simple step changes. This is the
 * simplest, but can lead to jerkier vehicle behavior.
 */
class SquareTrajectory : public TrajectoryGenerator {
public:
    SquareTrajectory(TrajectoryOptions& options)
        : TrajectoryGenerator(options) {}

    virtual bool generate(double arc_length, double curvature, Trajectory& trajectory) override;
};

/**
 * @brief This class will eventually go-away, replaced with the
 * trapezoidal-trajectory generation with the appropriate blending.
 */
class SquareTrajectoryWithDeceleration : public SquareTrajectory {
public:
    SquareTrajectoryWithDeceleration(TrajectoryOptions& options)
        : SquareTrajectory(options) {}

    bool generate(double arc_length, double curvature, Trajectory& trajectory) override;
};

/**
 * @brief Velocity set-point changes are interpolated through specified
 * acceleration limits.
 */
class TrapezoidalTrajectory : public TrajectoryGenerator {
public:
    TrapezoidalTrajectory(TrajectoryOptions& options);

    bool generate(double arc_length, double curvature, Trajectory& trajectory) override;

private:
    class TrapezoidalTrajectoryImpl;
    std::shared_ptr<TrapezoidalTrajectoryImpl> m_impl;
};

} // planning
