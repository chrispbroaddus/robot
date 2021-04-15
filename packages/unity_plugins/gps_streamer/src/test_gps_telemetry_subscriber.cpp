#include "glog/logging.h"
#include "packages/hal/proto/gps_telemetry.pb.h"
#include "packages/net/include/zmq_topic_sub.h"

#include <cstdlib>
#include <zmq.hpp>

///
/// @brief Run the camera subscriber for testing purposes
///
int main(int argc, char** argv) {

    google::InitGoogleLogging(argv[0]);

    if (argc < 3) {
        LOG(ERROR) << "USAGE: " << argv[0] << " IP_ADDRESS PORT_NUMBER";
    }

    std::string ipAddress(argv[1]);
    std::string portStr(argv[2]);

    //  Prepare our context and subscriber
    zmq::context_t context(1);
    std::string addr = "tcp://" + ipAddress + ":" + portStr;
    net::ZMQProtobufSubscriber<hal::GPSTelemetry> subscriber(context, addr, "gps", 1);

    LOG(INFO) << "Subscriber connected.";

    while (1) {
        hal::GPSTelemetry gpsTelemetry;
        if (subscriber.recv(gpsTelemetry)) {
            LOG(INFO) << "Timestamp : " << gpsTelemetry.timestamp().nanos();
            LOG(INFO) << "  Location : " << gpsTelemetry.latitude() << "°, " << gpsTelemetry.longitude() << "°, " << gpsTelemetry.altitude()
                      << "m";
            LOG(INFO) << "  HDOP :" << gpsTelemetry.horizontal_dilution_of_precision();
            LOG(INFO) << "  # Sats :" << gpsTelemetry.num_of_satellites_in_view();
        }
    }
    return EXIT_SUCCESS;
}
