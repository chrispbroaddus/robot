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

DEFINE_string(log_file, "", "Log file to read");

namespace planning {

using planning::readLog;
using std::chrono::nanoseconds;
using core::chrono::gps::wallClockInNanoseconds;

nanoseconds starttimeFromEnvelope(const hal::VCUCommandEnvelope& envelope) {
    return nanoseconds(envelope.trajectorycommand().segments(0).targetstarttime().nanos());
}

void adjustTiming(std::deque<hal::VCUCommandEnvelope>& commands, std::chrono::nanoseconds offset, std::chrono::nanoseconds startupDelay) {
    nanoseconds start{ 0 };

    // Make relative
    for (auto& command : commands) {
        for (auto& segment : *command.mutable_trajectorycommand()->mutable_segments()) {
            auto segmentTime = nanoseconds(segment.targetstarttime().nanos());
            if (start.count() == 0) {
                start = segmentTime;
            }
            segment.mutable_targetstarttime()->set_nanos((segmentTime - start).count());
        }
    }
    CHECK(start.count() != 0);
    // Adjust
    for (auto& command : commands) {
        for (auto& segment : *command.mutable_trajectorycommand()->mutable_segments()) {
            auto segmentTime = nanoseconds(segment.targetstarttime().nanos());
            segment.mutable_targetstarttime()->set_nanos((segmentTime + offset + startupDelay).count());
        }
    }
}

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("Playback a navigation log");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    FLAGS_logtostderr = true;
    CHECK(!FLAGS_log_file.empty()) << "Require logfile";

    std::deque<hal::VCUCommandEnvelope> log;
    CHECK(readLog(FLAGS_log_file, log));
    CHECK(!log.empty());
    CHECK(std::is_sorted(log.begin(), log.end(), [](const hal::VCUCommandEnvelope& lhs, const hal::VCUCommandEnvelope& rhs) {
        return starttimeFromEnvelope(lhs) < starttimeFromEnvelope(rhs);
    }));
    // Load default executor options:
    //      config/global/executor_options.default.pbtxt
    executor::ExecutorOptions executor_options = executor::loadDefaultExecutorOptions();
    std::stringstream fullyQualifiedAddress;
    fullyQualifiedAddress << executor_options.address() << ":" << executor_options.port();
    typedef net::ZMQProtobufReqClient<hal::VCUCommandEnvelope, hal::VCUCommandResponse> CommsType;
    constexpr int kDefaultTimeoutMs = 1000;
    constexpr int kLingerMs = 500;
    zmq::context_t context = zmq::context_t(1);
    std::unique_ptr<CommsType> comms_ptr(new CommsType(context, fullyQualifiedAddress.str(), kDefaultTimeoutMs, kLingerMs));

    LOG(INFO) << "Replaying: " << log.size() << " messages";

    constexpr int kDefaultStartupDelay = 2;
    nanoseconds startupDelay = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(kDefaultStartupDelay));

    nanoseconds playbackStart = wallClockInNanoseconds();
    adjustTiming(log, playbackStart, startupDelay);

    while (!log.empty()) {
        auto message = log.front();
        auto timestamp = starttimeFromEnvelope(message);
        auto now = wallClockInNanoseconds();
        // Publish if we are within 1 ms
        if (timestamp - now < std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1))) {
            LOG(INFO) << message.DebugString();
            CHECK(comms_ptr->send(message));
            log.pop_front();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return EXIT_SUCCESS;
}

} // planning

int main(int argc, char* argv[]) { return planning::main(argc, argv); }
