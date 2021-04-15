#include "packages/net/include/zmq_req_client.h"
#include "packages/net/include/zmq_topic_sub.h"

#include "packages/hal/proto/vcu_command_envelope.pb.h"
#include "packages/hal/proto/vcu_command_response.pb.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"

#include "glog/logging.h"

#include <thread>

using namespace hal;
using namespace net;

int main() {

    zmq::context_t context = zmq::context_t(1);
    LOG(INFO) << "Creating Subscriber\n";
    ZMQProtobufSubscriber<VCUTelemetryEnvelope> telemetrySub(context, "tcp://localhost:5801", "telemetry", 1);
    LOG(INFO) << "Creating client\n";
    ZMQProtobufReqClient<hal::VCUCommandEnvelope, hal::VCUCommandResponse> client(context, "tcp://localhost:5802", 1000, 500);

    LOG(INFO) << "Starting transmission\n";
    int counter = 0;
    while (1) {
        VCUTelemetryEnvelope telemetrySample;
        if (telemetrySub.poll()) {
            if (telemetrySub.recv(telemetrySample)) {
                LOG(INFO) << "Received sample (in nanosec): " << telemetrySample.sendtimestamp().nanos() << std::endl;
            }
        }

        hal::VCUCommandEnvelope trajectory;
        hal::VCUCommandResponse trajectoryResponse;

        trajectory.default_instance();
        trajectory.set_sequencenumber(counter++);

        if (client.send(trajectory)) {
            if (client.recv(trajectoryResponse)) {
                LOG(INFO) << "Send trajectory " << counter << " successful" << std::endl;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
