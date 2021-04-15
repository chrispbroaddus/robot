#include "glog/logging.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/net/include/zmq_topic_sub.h"

///
/// @brief Binary to test functionality of the VcuTelemetryPublisher.
///
int main(int argc, char** argv) {

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<hal::VCUTelemetryEnvelope> subscriber(context, "tcp://127.0.0.1:" + std::string(argv[1]), "telemetry", 1);

    while (true) {
        if (subscriber.poll(std::chrono::milliseconds(10))) {
            hal::VCUTelemetryEnvelope envelope;
            if (subscriber.recv(envelope)) {
                if (envelope.has_wheelencoder()) {
                    auto wheelTelemetry = envelope.wheelencoder();
                    LOG(INFO) << "Wheel ID : " << wheelTelemetry.wheelid();
                    LOG(INFO) << " Dist : " << wheelTelemetry.lineardisplacementmeters();
                    LOG(INFO) << " Timestamp : " << wheelTelemetry.periodendsystemtimestamp().nanos();
                }
                if (envelope.has_slider()) {
                    auto slider = envelope.slider();
                    LOG(INFO) << "Slider ID : " << slider.sliderid();
                    LOG(INFO) << " position : " << slider.positioninmeters();
                    LOG(INFO) << " Timestamp : " << slider.measurementsystemtimestamp().nanos();
                }
                if (envelope.has_servo()) {
                    auto servo = envelope.servo();
                    LOG(INFO) << "Servo ID : " << servo.servoid();
                    LOG(INFO) << " angle(radian) : " << servo.servoangleradians();
                    LOG(INFO) << " Timestamp : " << servo.measurementsystemtimestamp().nanos();
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
