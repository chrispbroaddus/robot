#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/core/include/chrono.h"
#include "packages/hal/proto/gps_telemetry.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/unity_plugins/connections_reader/include/connections_reader.h"

namespace {
constexpr auto DEFAULT_CONNECTIONS_FILE = "packages/unity_simulator/test/connections.default.json";
constexpr int REQUIRED_MESSAGE_COUNT = 3;
}

///
/// @brief Checks that GPS data is received
/// @details Subscribes to the GPS data channel and checks valid data is received
///
TEST(IMUDataValidation, checkReceiving) {

    unity_plugins::ConnectionsReader reader(DEFAULT_CONNECTIONS_FILE);
    const auto address = reader.fullAddress(mercury::topic::gps);

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<hal::GPSTelemetry> subscriber(context, address, "gps", 1);

    auto startTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsedTime = std::chrono::duration<double, std::milli>::zero();

    int receivedMessageCount = 0;
    while (elapsedTime < std::chrono::seconds(5)) {

        if (subscriber.poll(std::chrono::milliseconds(10))) {

            hal::GPSTelemetry gpsTelemetry;
            if (subscriber.recv(gpsTelemetry)) {

                receivedMessageCount++;

                if (receivedMessageCount > REQUIRED_MESSAGE_COUNT) {
                    break;
                }
            }
        }
        elapsedTime = std::chrono::high_resolution_clock::now() - startTime;
    }

    EXPECT_TRUE(receivedMessageCount > REQUIRED_MESSAGE_COUNT);
}
