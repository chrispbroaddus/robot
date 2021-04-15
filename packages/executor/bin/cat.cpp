#include "gflags/gflags.h"

#include "packages/executor/logging.h"
#include "packages/planning/logging.h"

DEFINE_string(logfile, "", "Logfile to display");

using planning::readLog;
using executor::LogEntry;

int main(int argc, char** argv) {
    gflags::SetUsageMessage("\"cat\" a log file to stdout");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = true;

    std::deque<LogEntry> entries;
    CHECK(!FLAGS_logfile.empty());
    CHECK(readLog(FLAGS_logfile, entries));

    for (auto entry : entries) {
        LOG(INFO) << entry.DebugString();
    }
    gflags::ShutDownCommandLineFlags();
}
