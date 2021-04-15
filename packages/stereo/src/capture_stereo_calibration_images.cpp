#include "packages/stereo/include/stereo_calibration_pipeline.h"

#include "gflags/gflags.h"
#include "glog/logging.h"

#include <atomic>
#include <iostream>
#include <signal.h>
#include <thread>

DEFINE_string(leftServerAddress, "", "left server address");
DEFINE_string(leftTopic, "", "left topic");
DEFINE_string(rightServerAddress, "", "right server address");
DEFINE_string(rightTopic, "", "right topic");
DEFINE_string(outputFolder, "", "output folder path");

std::atomic<bool> s_stopStereoDemo(false);
void SIGINTHandler(int /*signum*/) { s_stopStereoDemo = true; }

int main(int argc, char* argv[]) {

    signal(SIGINT, SIGINTHandler);
    google::InitGoogleLogging(argv[0]);

    gflags::SetUsageMessage("Stereo Calibration Utility");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    if (FLAGS_leftServerAddress.empty() || FLAGS_leftTopic.empty() || FLAGS_rightServerAddress.empty() || FLAGS_rightTopic.empty()
        || FLAGS_outputFolder.empty()) {
        gflags::ShowUsageWithFlagsRestrict(argv[0], "packages/stereo/src/stereo_demo_main.cpp");
        return 0;
    }

    stereo::StereoCalibrationPipeline stereoCalibrationPipeline(
        FLAGS_leftServerAddress, FLAGS_leftTopic, FLAGS_rightServerAddress, FLAGS_rightTopic, FLAGS_outputFolder);
    stereoCalibrationPipeline.start();

    while (!s_stopStereoDemo) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    stereoCalibrationPipeline.stop();
    gflags::ShutDownCommandLineFlags();

    return 0;
}
