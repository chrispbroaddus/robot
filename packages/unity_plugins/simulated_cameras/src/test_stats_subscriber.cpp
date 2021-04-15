//
// Created by byungsookim on 5/14/17.
//

#include "glog/logging.h"
#include "packages/hal/proto/simulator_stats_telemetry.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/unity_plugins/simulated_cameras/include/simulator_publisher_topics.h"

#include <cstdlib>
#include <zmq.hpp>

using namespace unity_plugins;

///
/// @brief Run the stats subscriber for testing purposes
///
int main(int argc, char** argv) {

    google::InitGoogleLogging(argv[0]);

    if (argc < 3) {
        LOG(ERROR) << "USAGE: " << argv[0] << " IP_ADDRESS PORT_NUMBER";
        return EXIT_FAILURE;
    }

    std::string ipAddress(argv[1]);
    std::string portStr(argv[2]);

    //  Prepare our context and subscriber
    zmq::context_t context(1);
    std::string addr = "tcp://" + ipAddress + ":" + portStr;
    net::ZMQProtobufSubscriber<hal::SimulatorStatsTelemetry> subscriber(context, addr, STATS_TOPIC, 1);

    LOG(INFO) << "subscriber connected.";

    while (1) {
        hal::SimulatorStatsTelemetry stats;
        if (subscriber.recv(stats)) {
            LOG(INFO) << "frame " << stats.framenumber() << ": " << stats.deltatimems() << "ms (" << (1000.0f / stats.deltatimems())
                      << " fps)";
        }
    }
    return 0;
}
