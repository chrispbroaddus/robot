#include "gflags/gflags.h"
#include "glog/logging.h"

#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/perception/logging.h"

#include <atomic>
#include <signal.h>
#include <thread>

DEFINE_string(depthSubscriberAddress, "tcp://localhost:5559", "Address to subscribe to");
DEFINE_string(depthTopic, "depth", "depth topic");
DEFINE_string(logOutput, "", "logging output");

static std::atomic<bool> s_stopStereoDemo(false);

void SIGINTHandler(__attribute__((unused)) int signum) { s_stopStereoDemo = true; }

int main(int argc, char* argv[]) {
    signal(SIGINT, SIGINTHandler);
    google::InitGoogleLogging(argv[0]);
    gflags::SetUsageMessage("Stereo logger");
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    FLAGS_alsologtostderr = 1;

    LOG(INFO) << FLAGS_depthSubscriberAddress << ", " << FLAGS_depthTopic;

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<hal::CameraSample> subscriber(context, FLAGS_depthSubscriberAddress, FLAGS_depthTopic, 1);

    std::string filename;
    if (FLAGS_logOutput.empty()) {
        char file_template[] = "/tmp/perception_stereologger_XXXXXX";
        int descriptor = mkstemp(file_template);
        CHECK(descriptor >= 0);
        filename = file_template;
    } else {
        filename = FLAGS_logOutput;
    }

    int logggedMessages = 0;
    {
        perception::LogWriter<hal::CameraSample> writer(filename);
        while (!s_stopStereoDemo) {
            if (subscriber.poll()) {
                hal::CameraSample sample;
                CHECK(subscriber.recv(sample));
                CHECK(writer.write(sample));
                ++logggedMessages;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    LOG(INFO) << "Wrote: " << logggedMessages << "messages";

    gflags::ShutDownCommandLineFlags();
    return EXIT_SUCCESS;
}
