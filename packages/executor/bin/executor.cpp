#include "packages/executor/executor.h"
#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/estimation/estimator.h"
#include "packages/executor/proto/executor_options.pb.h"
#include "packages/executor/proto_helpers.h"
#include "packages/perception/perception.h"
#include "packages/perception/proto/perception_options.pb.h"
#include "packages/planning/planner.h"
#include "packages/planning/proto/trajectory_planner_options.pb.h"

#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_string(mode, "", "Mode to execute (xy: give direct x(m) y(m) goals to the executor, camera: give PointAndGoDirectives to the "
                        "executor x& y interpreted as relative pixel coordinates)");
DEFINE_double(x, 0.0, "X goal (m)");
DEFINE_double(y, 0.0, "Y goal (m)");

DEFINE_string(system_calibration, "", "System calibration (if applicable)");
DEFINE_string(camera, "", "Camera to use (if applicable)");

namespace perception {
class BasicPerception : public Perception {
public:
    BasicPerception(const PerceptionOptions& options, std::unique_ptr<TerrainRepresentation> terrain, const std::string& system_calibration)
        : Perception(options, std::move(terrain)) {
        calibration::SystemCalibration calibration;
        CHECK(planning::loadOptions(system_calibration, calibration, planning::SerializationType::JSON));
        CHECK(this->addDevices(calibration));
    }
};
} // perception

namespace estimation {
class DefaultEstimator : public Estimator {
public:
    DefaultEstimator(__attribute__((unused)) const EstimatorOptions& options) {}
};
}

namespace planning {

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    CHECK(!FLAGS_mode.empty()) << "Require mode to be set";
    CHECK(!FLAGS_system_calibration.empty()) << "Require system calibration";

    using estimation::Estimator;
    using estimation::DefaultEstimator;
    using estimation::EstimatorOptions;
    using executor::Executor;
    using executor::ExecutorOptions;
    using executor::loadDefaultExecutorOptions;
    using perception::BasicPerception;
    using perception::FlatTerrain;
    using perception::Perception;
    using perception::PerceptionOptions;

    EstimatorOptions estimator_options = estimation::loadDefaultEstimatorOptions();
    ExecutorOptions executor_options = loadDefaultExecutorOptions();
    ArcPlannerOptions planner_options = loadDefaultArcPlannerOptions();
    TrajectoryOptions trajectory_options = loadDefaultTrajectoryOptions();

    std::unique_ptr<Estimator> estimator_ptr(new DefaultEstimator(estimator_options));
    std::unique_ptr<TrajectoryGenerator> generator_ptr(new SquareTrajectoryWithDeceleration(trajectory_options));
    std::shared_ptr<TrajectoryPlanner> planner_ptr(new ArcPlanner(planner_options, std::move(generator_ptr)));

    std::unique_ptr<StateMachineAdapter> planner_adapter_ptr;
    planner_adapter_ptr.reset(new StateMachineAdapter(planner_ptr));

    PerceptionOptions perception_options;
    std::unique_ptr<FlatTerrain> terrain(new FlatTerrain);
    std::unique_ptr<Perception> perception_ptr(new BasicPerception(perception_options, std::move(terrain), FLAGS_system_calibration));
    std::unique_ptr<Comms> comms_ptr(new ZMQComms(executor_options.address(), executor_options.port(), true));

    std::unique_ptr<Executor> executor_ptr;
    executor_ptr.reset(new Executor(
        executor_options, std::move(estimator_ptr), std::move(planner_adapter_ptr), std::move(perception_ptr), std::move(comms_ptr)));

    if ("xy" == FLAGS_mode) {
        core::Point3d target;
        target.set_x(FLAGS_x);
        target.set_y(FLAGS_y);
        return executor_ptr->execute(target);
    } else if ("camera" == FLAGS_mode) {
        CHECK(!FLAGS_camera.empty()) << "Require camera to query";
        teleop::PointAndGoCommand command;
        command.set_camera(FLAGS_camera);
        command.set_imagex(FLAGS_x);
        command.set_imagey(FLAGS_y);
        return executor_ptr->execute(command);
    } else {
        throw std::runtime_error("Unknown mode!");
    }
}

} // planning

int main(int argc, char** argv) { planning::main(argc, argv); }
