#include "packages/net/include/zmq_rep_server.h"
#include "packages/net/include/zmq_topic_pub.h"

#include "packages/hal/proto/vcu_command_envelope.pb.h"
#include "packages/hal/proto/vcu_command_response.pb.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"

#include "glog/logging.h"
#include "packages/planning/utils.h"

#include <iostream>
#include <thread>

using namespace hal;
using namespace net;

int main(int, char**) {
    zmq::context_t context = zmq::context_t(1);
    ZMQProtobufPublisher<VCUTelemetryEnvelope> telemetryPub(context, "tcp://*:5804", 1, 1000);
    ZMQProtobufRepServer<hal::VCUCommandEnvelope, hal::VCUCommandResponse> repServer(context, "tcp://*:5803", 1000, 500);

    int counter = 0;
    while (1) {
        VCUTelemetryEnvelope telemetrySample;
        telemetrySample.default_instance();
        telemetrySample.mutable_sendtimestamp()->set_nanos(std::chrono::nanoseconds(std::chrono::seconds(counter)).count());
        counter++;
        telemetryPub.send(telemetrySample, "telemetry");

        hal::VCUCommandEnvelope trajectory;
        hal::VCUCommandResponse trajectoryResponse;

        if (repServer.recv(trajectory)) {
            LOG(INFO) << "Received trajectory: " << trajectory.sequencenumber() << std::endl;
            trajectoryResponse.set_disposition(hal::CommandAccepted);
            repServer.send(trajectoryResponse);
        }

        LOG(INFO) << "Sent count: " << counter << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
