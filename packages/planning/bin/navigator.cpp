#include "packages/planning/navigator.h"
#include "packages/estimation/estimator.h"
#include "packages/executor/proto/executor_options.pb.h"
#include "packages/executor/proto_helpers.h"
#include "packages/perception/perception.h"
#include "packages/perception/proto/perception_options.pb.h"
#include "packages/planning/planner.h"
#include "packages/planning/proto/trajectory_planner_options.pb.h"

#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_double(x, 0.0, "X goal (m)");
DEFINE_double(y, 0.0, "Y goal (m)");

namespace perception {
class DefaultPerception : public Perception {
public:
    DefaultPerception(const PerceptionOptions& options)
        : Perception(options, std::unique_ptr<TerrainRepresentation>()) {}
};
} // perception

namespace planning {

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    using estimation::GroundTruthEstimator;
    using estimation::GroundTruthEstimatorOptions;
    using estimation::EstimatorOptions;
    using executor::ExecutorOptions;
    using perception::Perception;
    using perception::PerceptionOptions;
    using perception::DefaultPerception;

    ExecutorOptions executor_options = executor::loadDefaultExecutorOptions();
    ArcPlannerOptions planner_options = loadDefaultArcPlannerOptions();
    TrajectoryOptions trajectory_options = loadDefaultTrajectoryOptions();

    GroundTruthEstimatorOptions ground_truth_options;
    ground_truth_options.mutable_base_options()->set_subscribeaddress("tcp://localhost");
    ground_truth_options.mutable_base_options()->set_subscribeport(7501);
    ground_truth_options.mutable_base_options()->set_publishaddress("tcp://127.0.0.1");
    ground_truth_options.mutable_base_options()->set_publishport(7101);

    std::shared_ptr<GroundTruthEstimator> estimator_ptr;
    estimator_ptr.reset(new GroundTruthEstimator(ground_truth_options));
    std::unique_ptr<TrajectoryGenerator> generator_ptr(new SquareTrajectory(trajectory_options));
    std::unique_ptr<ArcPlanner> trajectory_ptr(new ArcPlanner(planner_options, std::move(generator_ptr)));
    std::unique_ptr<TrajectoryPlanner> trajectory_wrapper_ptr(new ConstrainedArcWrapper(std::move(trajectory_ptr)));

    std::unique_ptr<Perception> perception_ptr(new DefaultPerception(PerceptionOptions()));
    std::unique_ptr<Comms> comms_ptr(new ZMQComms(executor_options.address(), executor_options.port(), true));

    SBPLPlannerOptions options;
    options.set_planner_type(SBPLPlannerOptions::ARASTAR);
    std::unique_ptr<SBPLPlanner> path_ptr;
    path_ptr.reset(new SBPLPlanner(options));

    using planning::EuclideanDistanceDecisionFunction;
    std::unique_ptr<GoalDecisionFunction> decision_ptr;
    decision_ptr.reset(new EuclideanDistanceDecisionFunction);

    NavigatorOptions navigator_options;
    navigator_options.set_frequency(20);

    std::unique_ptr<WaypointNavigator> navigator_ptr;
    navigator_ptr.reset(new WaypointNavigator(navigator_options, std::move(comms_ptr), std::move(trajectory_wrapper_ptr),
        std::move(path_ptr), estimator_ptr, std::move(decision_ptr)));
    Sophus::SE3d target;
    CHECK(FLAGS_x > 0);
    target.translation()[planning::kXAxis] = FLAGS_x;
    target.translation()[planning::kYAxis] = FLAGS_y;

    auto future = navigator_ptr->navigateTo(target);
    return future.get() == true ? EXIT_SUCCESS : EXIT_FAILURE;
}

} // planning

int main(int argc, char** argv) { planning::main(argc, argv); }
