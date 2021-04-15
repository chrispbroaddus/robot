#include "glog/logging.h"
#include "packages/docking/proto/docking_station.pb.h"
#include "packages/net/include/zmq_topic_sub.h"

///
/// @brief Binary to test functionality of the FiducialStationFinder.
///
int main(int argc, char** argv) {

    if (argc < 2) {
        LOG(INFO) << "Usage : ./test_docking_station_list_subscriber PORT_NUMBER_TO_SUBSCRIBE";
        return EXIT_FAILURE;
    }

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<docking::DockingStationList> subscriber(
        context, "tcp://localhost:" + std::string(argv[1]), "docking_observation", 1);

    while (true) {
        if (subscriber.poll(std::chrono::milliseconds(1))) {
            docking::DockingStationList list;
            subscriber.recv(list);
            LOG(INFO) << list.docking_stations_size();
        }
    }

    return EXIT_SUCCESS;
}
