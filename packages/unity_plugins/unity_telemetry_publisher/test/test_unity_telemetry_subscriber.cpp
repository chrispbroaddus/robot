#include "glog/logging.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/unity_plugins/proto/unity_telemetry_envelope.pb.h"

///
/// @brief Binary to test functionality of the WheelTelemetryPublisher.
///
int main(int argc, char** argv) {

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<unity_plugins::UnityTelemetryEnvelope> subscriber(
        context, "tcp://127.0.0.1:" + std::string(argv[1]), "telemetry", 1);

    while (true) {
        if (subscriber.poll(std::chrono::milliseconds(10))) {
            unity_plugins::UnityTelemetryEnvelope envelope;
            if (subscriber.recv(envelope)) {
                if (envelope.has_vehiclepose()) {
                    auto vehiclepose = envelope.vehiclepose();
                    LOG(INFO) << "Timestamp : " << vehiclepose.measurementsystemtimestamp().nanos();
                    LOG(INFO) << " x : " << vehiclepose.transformations().translationx();
                    LOG(INFO) << " y : " << vehiclepose.transformations().translationy();
                    LOG(INFO) << " z : " << vehiclepose.transformations().translationz();
                    LOG(INFO) << " rx : " << vehiclepose.transformations().rodriguesrotationx();
                    LOG(INFO) << " ry : " << vehiclepose.transformations().rodriguesrotationy();
                    LOG(INFO) << " rz : " << vehiclepose.transformations().rodriguesrotationz();
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
