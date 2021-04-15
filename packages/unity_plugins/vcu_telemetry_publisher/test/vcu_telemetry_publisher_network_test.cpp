#include "glog/logging.h"
#include "gtest/gtest.h"

#include <atomic>
#include <cmath>
#include <thread>

#include "packages/net/include/zmq_topic_sub.h"
#include "packages/unity_plugins/vcu_telemetry_publisher/include/vcu_telemetry_publisher.h"

using namespace unity_plugins;

namespace {
constexpr auto s_address = "tcp://127.0.0.1:7500";
constexpr auto s_topic = "telemetry";
const int s_lingerPeriodInMilliseconds = 1000;
const int s_highWaterMark = 1;
const VcuTelemetryPublisher::WheelsRadius s_wheelsRadius{ 0.10, 0.11, 0.12, 0.13, 0.14, 0.15 };
}

class VcuTelemetryPublisherTest : public testing::Test {
protected:
    VcuTelemetryPublisherTest() {}

    void SetUp() {}
};

void testPublisher(const hal::VCUWheelID wheelId, const int durationInNanosecond, const float angle, std::atomic_bool& stop) {

    VcuTelemetryPublisher publisher(s_address, s_highWaterMark, s_lingerPeriodInMilliseconds, s_wheelsRadius);

    while (stop == false) {
        publisher.sendWheelTelemetry(wheelId, durationInNanosecond, angle);
    }
};

TEST_F(VcuTelemetryPublisherTest, received_data_correctness) {

    for (int i = 0; i < 6; i++) {

        hal::VCUWheelID wheelId = (hal::VCUWheelID)i;

        std::atomic_bool stop{ false };

        constexpr int durationInNanosecond = 100000;
        constexpr float distance = 0.1;

        std::thread thread(testPublisher, wheelId, durationInNanosecond, distance, std::ref(stop));

        zmq::context_t context(1);
        net::ZMQProtobufSubscriber<hal::VCUTelemetryEnvelope> subscriber(context, s_address, s_topic, s_highWaterMark);

        EXPECT_TRUE(subscriber.poll(std::chrono::milliseconds(1000)));

        hal::VCUTelemetryEnvelope envelope;
        EXPECT_TRUE(subscriber.recv(envelope));
        EXPECT_TRUE(envelope.has_wheelencoder());
        EXPECT_EQ(envelope.wheelencoder().wheelid(), wheelId);
        EXPECT_EQ(envelope.wheelencoder().lineardisplacementmeters(), distance);

        stop = true;
        thread.join();
    }
}
