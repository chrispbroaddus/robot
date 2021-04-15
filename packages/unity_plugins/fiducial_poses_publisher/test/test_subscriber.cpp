#include "glog/logging.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/perception/fiducials/proto/fiducial_poses.pb.h"

int main(int argc, char** argv) {
    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<perception::FiducialPoses> subscriber(
        context, "tcp://127.0.0.1:" + std::string(argv[1]), "fiducial_poses", 1);

    while (1) {
        perception::FiducialPoses rPoses;
        if (subscriber.poll(std::chrono::milliseconds(1))) {
            subscriber.recv(rPoses);
            LOG(INFO) << "rPoses.transformations_size(): " << rPoses.transformations_size();
        } else {
            LOG(INFO) << "The polling returned false.";
        }
    }

    return EXIT_SUCCESS;
}
