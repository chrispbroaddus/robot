
#include "gflags/gflags.h"
#include "glog/logging.h"

#include "packages/videoviewer/include/video_viewer.h"

#include <atomic>
#include <signal.h>
#include <thread>

DEFINE_string(address, "", "ZMQ server address such as: tcp://localhost:5556");
DEFINE_string(topic, "", "ZMQ topic such as: camera");

static std::atomic_bool s_stopViewer(false);

void SIGINTHandler(int /*signum*/) { s_stopViewer = true; }

int main(int argc, char* argv[]) {

    signal(SIGINT, SIGINTHandler);

    google::InitGoogleLogging(argv[0]);

    gflags::SetUsageMessage("Video viewer");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    if (FLAGS_address.empty() || FLAGS_topic.empty()) {
        gflags::ShowUsageWithFlagsRestrict(argv[0], "packages/video_viewer/src/main.cpp");
        return 0;
    }

    LOG(INFO) << "address: " << FLAGS_address;
    LOG(INFO) << "topic: " << FLAGS_topic;

    video_viewer::VideoViewer viewer(FLAGS_address, FLAGS_topic);

    while (!s_stopViewer) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    gflags::ShutDownCommandLineFlags();

    return 0;
}
