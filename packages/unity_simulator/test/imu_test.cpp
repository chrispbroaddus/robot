#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/core/include/chrono.h"
#include "packages/hal/proto/imu_sample.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/unity_plugins/connections_reader/include/connections_reader.h"

namespace {
constexpr auto DEFAULT_CONNECTIONS_FILE = "packages/unity_simulator/test/connections.default.json";
constexpr int REQUIRED_MESSAGE_COUNT = 10;
}

///
/// @brief Checks that IMU data is received
/// @details Subscribes to the imu data channel and checks valid data is received
///
TEST(IMUDataValidation, checkReceiving) {

    unity_plugins::ConnectionsReader reader(DEFAULT_CONNECTIONS_FILE);
    const auto address = reader.fullAddress(mercury::topic::imu);

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<hal::IMUSample> subscriber(context, address, "imu", 1);

    auto startTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsedTime = std::chrono::duration<double, std::milli>::zero();

    int receivedMessageCount = 0;
    while (elapsedTime < std::chrono::seconds(5)) {

        if (subscriber.poll(std::chrono::milliseconds(10))) {

            hal::IMUSample imuSample;
            if (subscriber.recv(imuSample)) {

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

TEST(IMUDataValidation, checkTimestamps) {
    unity_plugins::ConnectionsReader reader(DEFAULT_CONNECTIONS_FILE);
    const auto address = reader.fullAddress(mercury::topic::imu);

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<hal::IMUSample> subscriber(context, address, "imu", 1);

    auto startTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsedTime = std::chrono::duration<double, std::milli>::zero();

    int receivedMessageCount = 0;

    uint64_t lastSystemTimestamp = 0;
    uint64_t deltaSystemTime = 0;
    uint64_t lastHardwareTimestamp = 0;
    uint64_t deltaHardwareTime = 0;

    while (elapsedTime < std::chrono::seconds(5)) {

        if (subscriber.poll(std::chrono::milliseconds(10))) {

            hal::IMUSample imuSample;
            if (subscriber.recv(imuSample)) {

                receivedMessageCount++;

                if (receivedMessageCount > REQUIRED_MESSAGE_COUNT) {
                    break;
                }

                auto systemTimestamp = imuSample.systemtimestamp().nanos();
                auto hardwareTimestamp = imuSample.hardwaretimestamp().nanos();

                deltaSystemTime = systemTimestamp - lastSystemTimestamp;
                deltaHardwareTime = hardwareTimestamp - lastHardwareTimestamp;

                EXPECT_GT(systemTimestamp, lastSystemTimestamp);
                EXPECT_GT(hardwareTimestamp, lastHardwareTimestamp);

                EXPECT_GT(deltaSystemTime, 0);
                EXPECT_GT(deltaHardwareTime, 0);

                lastSystemTimestamp = systemTimestamp;
                lastHardwareTimestamp = hardwareTimestamp;
            }
        }
        elapsedTime = std::chrono::high_resolution_clock::now() - startTime;
    }

    EXPECT_TRUE(receivedMessageCount > REQUIRED_MESSAGE_COUNT);
}
