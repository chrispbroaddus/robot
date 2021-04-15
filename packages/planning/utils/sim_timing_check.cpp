#include "packages/executor/proto/executor_options.pb.h"
#include "packages/executor/proto_helpers.h"
#include "packages/planning/utils.h"

#include "gflags/gflags.h"
#include "glog/logging.h"

std::chrono::milliseconds frequencyToMilliseconds(float frequency) {
    return std::chrono::milliseconds(static_cast<int>(1 / frequency * 1000));
}

int main(int argc, char** argv) {
    gflags::SetUsageMessage("Sim offset checker");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    using executor::ExecutorOptions;
    using executor::loadDefaultExecutorOptions;
    using planning::ZMQComms;
    using planning::Trajectory;
    using planning::relativeTrajectoryToAbsolute;

    ExecutorOptions executor_options = executor::loadDefaultExecutorOptions();
    ZMQComms comms(executor_options.address(), executor_options.port(), true);

    Trajectory absoluteTrajectory, relativeTrajectory;

    auto traverse = relativeTrajectory.add_elements();
    core::Duration traverseDuration;
    traverseDuration.set_nanos(10000);
    *traverse->mutable_relative_time() = traverseDuration;

    constexpr int kOffsetInNanoseconds = 5e7;
    float frequency = 10;
    bool sustainedPublish = true;
    const int kNumberPublishedMessages = 20;
    int publishCounter = kNumberPublishedMessages;
    while (sustainedPublish) {
        relativeTrajectoryToAbsolute(relativeTrajectory, std::chrono::nanoseconds(kOffsetInNanoseconds), absoluteTrajectory);
        sustainedPublish = comms.send(absoluteTrajectory);
        std::this_thread::sleep_for(frequencyToMilliseconds(frequency));

        if (publishCounter-- == 0) {
            LOG(INFO) << "Frequency: " << frequency << " sustained";
            publishCounter = kNumberPublishedMessages;
            ++frequency;
        }
    }

    LOG(INFO) << "Max frequency: " << frequency;
    return EXIT_SUCCESS;
}
