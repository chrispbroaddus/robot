#include "gflags/gflags.h"
#include "glog/logging.h"

#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/perception/logging.h"
#include "packages/perception/types.h"
#include "packages/perception/utils.h"

#include <atomic>
#include <deque>
#include <signal.h>
#include <thread>

DEFINE_string(logInput, "", "logging input");

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    gflags::SetUsageMessage("Stereo reader");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    FLAGS_alsologtostderr = 1;

    CHECK(!FLAGS_logInput.empty()) << "Require log input";
    perception::LogReader<hal::CameraSample> reader(FLAGS_logInput);

    int readMessages = 0;
    hal::CameraSample sample;
    std::deque<hal::CameraSample> samples;
    while (reader.read(sample)) {
        samples.emplace_back(sample);

        switch (sample.image().format()) {
        case hal::Format::PB_RANGE:
            LOG(INFO) << "Read range image";
            break;
        case hal::Format::PB_POINTCLOUD:
            LOG(INFO) << "Read point cloud";
            break;
        default:
            LOG(FATAL) << "Unknown!";
        }

        perception::PointCloudXYZ cloud;
        cloud.xyz = Eigen::Map<const Eigen::MatrixXf>(
            reinterpret_cast<const float*>(sample.image().data().data()), 3, sample.image().rows() * sample.image().cols());
        const auto rows = sample.image().rows();
        const auto cols = sample.image().cols();
        auto expected_size = rows * cols * 3 * sizeof(float);
        LOG(INFO) << expected_size << " vs " << sample.image().data().size();
        LOG(INFO) << cloud.xyz.block<3, 1>(0, 0);
        perception::checkCloud(cloud, sample.image().rows() * sample.image().cols());
        ++readMessages;
    }

    LOG(INFO) << "Read " << readMessages << " messages";

    gflags::ShutDownCommandLineFlags();
    return EXIT_SUCCESS;
}
