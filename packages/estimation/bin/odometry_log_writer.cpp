#include "packages/estimation/estimator.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/planning/logging.h"

namespace estimation {

using planning::BaseLogger;

class EstimationLogger : public BaseLogger<hal::VCUTelemetryEnvelope> {
public:
    EstimationLogger(const std::string& log)
        : BaseLogger(log) {}
};

int main(int /* argc */, const char** /* argv[] */) {
    zmq::context_t context(1);
    constexpr int kDefaultPort = 7001;

    std::string address = "tcp://localhost:" + std::to_string(kDefaultPort);
    net::ZMQProtobufSubscriber<hal::VCUTelemetryEnvelope> subscriber(context, address, "telemetry", 1);
    EstimationLogger log("odometry.log");

    while (true) {
        if (subscriber.poll(std::chrono::milliseconds(10))) {
            hal::VCUTelemetryEnvelope envelope;
            if (subscriber.recv(envelope)) {
                LOG_EVERY_N(INFO, 100) << envelope.DebugString();
                log.add(envelope);
            }
        }
    }
    return EXIT_SUCCESS;
}

} // estimation

int main(int argc, const char* argv[]) { return estimation::main(argc, argv); }
