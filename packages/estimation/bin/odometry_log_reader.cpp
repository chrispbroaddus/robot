#include "gflags/gflags.h"
#include "packages/estimation/estimator.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/planning/logging.h"

DEFINE_string(log_file, "", "Log file to read");

namespace estimation {

using planning::readLog;

int main(int argc, char* argv[]) {
    gflags::SetUsageMessage("Read an odometry log");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    FLAGS_logtostderr = true;
    CHECK(!FLAGS_log_file.empty()) << "Require logfile";
    std::deque<hal::VCUTelemetryEnvelope> log;
    CHECK(readLog(FLAGS_log_file, log));
    CHECK(!log.empty());
    LOG(INFO) << "Read: " << log.size() << " messages";
    return EXIT_SUCCESS;
}

} // estimation

int main(int argc, char* argv[]) { return estimation::main(argc, argv); }
