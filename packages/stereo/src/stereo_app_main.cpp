
#include "gflags/gflags.h"
#include "glog/logging.h"

#include "packages/stereo/include/stereo_pipeline.h"

#include <atomic>
#include <iostream>
#include <signal.h>
#include <thread>

DEFINE_string(leftServerAddress, "", "left server address");
DEFINE_string(leftTopic, "", "left topic");
DEFINE_string(rightServerAddress, "", "right server address");
DEFINE_string(rightTopic, "", "right topic");
DEFINE_string(systemCalibrationFile, "", "system calibration file");
DEFINE_string(depthPublisherAddress, "tcp://*:5559", "depth publisher address");
DEFINE_string(depthTopic, "depth", "depth topic");
DEFINE_bool(outputPointCloud, false, "use true to output point-cloud data");

static std::atomic<bool> s_stopStereoDemo(false);

void SIGINTHandler(int /*signum*/) { s_stopStereoDemo = true; }

int main(int argc, char* argv[]) {
    signal(SIGINT, SIGINTHandler);
    google::InitGoogleLogging(argv[0]);

    gflags::SetUsageMessage("Stereo demo");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    if (FLAGS_leftServerAddress.empty() || FLAGS_leftTopic.empty() || FLAGS_rightServerAddress.empty() || FLAGS_rightTopic.empty()
        || FLAGS_systemCalibrationFile.empty()) {
        gflags::ShowUsageWithFlagsRestrict(argv[0], "packages/stereo/src/stereo_demo_main.cpp");
        return 0;
    }

    LOG(INFO) << "Depth data publisher address: " << FLAGS_depthPublisherAddress << " topic: " << FLAGS_depthTopic;

    stereo::StereoPipeline stereoPipeline(FLAGS_leftServerAddress, FLAGS_leftTopic, FLAGS_rightServerAddress, FLAGS_rightTopic,
        FLAGS_systemCalibrationFile, FLAGS_depthPublisherAddress, FLAGS_depthTopic, FLAGS_outputPointCloud);
    stereoPipeline.start();

    while (!s_stopStereoDemo) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    stereoPipeline.stop();
    gflags::ShutDownCommandLineFlags();

    return 0;
}
