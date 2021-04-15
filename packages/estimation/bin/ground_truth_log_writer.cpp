#include "packages/estimation/estimator.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/planning/logging.h"
#include "packages/unity_plugins/proto/unity_telemetry_envelope.pb.h"

#include "gflags/gflags.h"

namespace estimation {

using planning::BaseLogger;

class GroundTruthLogger : public BaseLogger<unity_plugins::UnityTelemetryEnvelope> {
public:
    GroundTruthLogger(const std::string& log)
        : BaseLogger(log) {}
};

int main(int /* argc */, const char** /*argv*/) {
    constexpr int kDefaultPort = 7501;

    zmq::context_t context(1);
    std::string address = "tcp://localhost:" + std::to_string(kDefaultPort);
    net::ZMQProtobufSubscriber<unity_plugins::UnityTelemetryEnvelope> subscriber(context, address, "", 1);
    GroundTruthLogger log("ground_truth.log");

    while (true) {
        if (subscriber.poll()) {
            unity_plugins::UnityTelemetryEnvelope envelope;
            if (subscriber.recv(envelope)) {
                LOG_EVERY_N(INFO, 10) << envelope.DebugString();
                log.add(envelope);
            }
        }
    }
    return EXIT_SUCCESS;
}

} // estimation

int main(int argc, const char** argv) { return estimation::main(argc, argv); }
