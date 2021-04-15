#include "glog/logging.h"
#include "packages/hal/proto/imu_sample.pb.h"
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
    net::ZMQProtobufSubscriber<hal::IMUSample> subscriber(context, addr, "imu", 1);

    LOG(INFO) << "Subscriber connected.";

    uint64_t lastTimestamp = 0;
    uint64_t deltaTime = 0;

    while (1) {
        hal::IMUSample imuSample;
        if (subscriber.recv(imuSample)) {
            LOG(INFO) << "System Timestamp : " << imuSample.systemtimestamp().nanos()
                      << ", Hardware Timestamp : " << imuSample.hardwaretimestamp().nanos();
            LOG(INFO) << "  Device : " << imuSample.device().name() << ", " << imuSample.device().serialnumber();
            LOG(INFO) << "  Gyro :" << imuSample.gyro(0) << ", " << imuSample.gyro(1) << ", " << imuSample.gyro(2);
            LOG(INFO) << "  Accel :" << imuSample.accel(0) << ", " << imuSample.accel(1) << ", " << imuSample.accel(2);
            LOG(INFO) << "  Mag :" << imuSample.mag(0) << ", " << imuSample.mag(1) << ", " << imuSample.mag(2);

            auto timestamp = imuSample.systemtimestamp().nanos();
            deltaTime = timestamp - lastTimestamp;
            lastTimestamp = timestamp;
            LOG(INFO) << "deltaTime : " << deltaTime;
        }
    }
    return EXIT_SUCCESS;
}
