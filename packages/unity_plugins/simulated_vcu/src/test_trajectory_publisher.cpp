#include "glog/logging.h"
#include "packages/core/include/chrono.h"
#include "packages/core/proto/timestamp.pb.h"
#include "packages/hal/proto/vcu_command_envelope.pb.h"
#include "packages/hal/proto/vcu_command_response.pb.h"
#include "packages/net/include/zmq_req_client.h"
#include <iostream>
#include <thread>

int main(int argc, char** argv) {

    google::InitGoogleLogging(argv[0]);

    if (argc != 4) {
        LOG(ERROR) << "Usage : " << argv[0] << " PORT VELOCITY RADIUS";
        return EXIT_SUCCESS;
    }
    int port = atoi(argv[1]);

    zmq::context_t context(1);
    std::string addr = "tcp://127.0.0.1:" + std::to_string(port);
    net::ZMQProtobufReqClient<hal::VCUCommandEnvelope, hal::VCUCommandResponse> client(context, addr, 1000, 1000);

    int counter = 0;
    while (1) {

        hal::VCUTrajectoryCommand* trajectory = new hal::VCUTrajectoryCommand();

        constexpr uint64_t gpsTimestampOffsetInNanos = 1e6;
        auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

        uint32_t millisecondPerElement = 500;
        int numElements = 20;

        float initialLinearVelocity = atof(argv[2]);
        float radius = atof(argv[3]);
        float inverseCurvature = 0;
        if (radius > 0) {
            inverseCurvature = 1.0f / radius;
        }

        for (int i = 0; i < numElements; i++) {
            auto element = trajectory->add_segments();

            uint64_t futureTimestamp
                = gpsTimestamp.count() + (uint64_t)i * millisecondPerElement * (uint64_t)1000000 + gpsTimestampOffsetInNanos;
            element->mutable_targetstarttime()->set_nanos(futureTimestamp);

            float linearVelocity = initialLinearVelocity * (1.0f - float(i) / float(numElements - 1));
            hal::VCUArcDriveSegment arcDriveSegment;
            arcDriveSegment.set_linearvelocitymeterspersecond(linearVelocity);
            arcDriveSegment.set_curvatureinversemeters(inverseCurvature);

            *element->mutable_arcdrive() = arcDriveSegment;

            LOG(INFO) << i << ": timestamp = " << std::to_string(futureTimestamp)
                      << ", velocity = " << arcDriveSegment.linearvelocitymeterspersecond()
                      << "m/s, inverse curvature = " << arcDriveSegment.curvatureinversemeters() << "m";
        }

        LOG(INFO) << "Sending out the data!";

        hal::VCUCommandEnvelope envelope;
        envelope.set_sequencenumber(++counter);
        envelope.set_allocated_trajectorycommand(trajectory);
        bool sendSuccess = client.send(envelope);

        hal::VCUCommandResponse response;
        client.recv(response);

        std::this_thread::sleep_for(std::chrono::milliseconds(7000));

        CHECK(sendSuccess);
    }

    return EXIT_SUCCESS;
}
