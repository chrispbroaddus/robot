#include "gflags/gflags.h"

#include "packages/core/include/chrono.h"
#include "packages/estimation/estimator.h"
#include "packages/executor/proto/executor_options.pb.h"
#include "packages/executor/proto_helpers.h"
#include "packages/hal/proto/vcu_command_envelope.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/planning/logging.h"
#include "packages/planning/proto/trajectory.pb.h"
#include "packages/planning/utils.h"

DEFINE_double(velocity, -1, "Velocity setpoint");
DEFINE_double(planner_frequency, -1, "Frequency (Hz)");

namespace planning {

Trajectory createTrajectory(float velocity, float curvature) {
    Trajectory trajectory;

    auto command = trajectory.add_elements();
    command->set_linear_velocity(velocity);
    command->set_curvature(curvature);

    core::Duration duration;
    duration.set_nanos(static_cast<int>(1e9));
    *command->mutable_relative_time() = duration;

    Trajectory absolute;
    relativeTrajectoryToAbsolute(trajectory, std::chrono::nanoseconds(0), absolute);
    return absolute;
}

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("Generate synthetic trajectories for sim");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    FLAGS_logtostderr = true;

    CHECK(FLAGS_velocity > 0) << "Invalid velocity";

    executor::ExecutorOptions executor_options = executor::loadDefaultExecutorOptions();
    std::unique_ptr<ZMQComms> comms_ptr(new ZMQComms(executor_options.address(), executor_options.port(), true));
    float curvature = 0.0;

    std::chrono::milliseconds delay(static_cast<int>(1 / FLAGS_planner_frequency * 1000));
    CHECK(delay.count() > 0);

    while (true) {
        curvature += 0.1f;
        auto publish = std::sin(curvature);
        auto trajectory = createTrajectory(FLAGS_velocity, publish);
        LOG(INFO) << publish;
        LOG(INFO) << trajectory.DebugString();
        CHECK(comms_ptr->send(trajectory));
        std::this_thread::sleep_for(delay);
    }
    return EXIT_SUCCESS;
}
}

int main(int argc, char* argv[]) { return planning::main(argc, argv); }
