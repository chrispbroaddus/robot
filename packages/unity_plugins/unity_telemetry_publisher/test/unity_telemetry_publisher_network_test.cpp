#include "glog/logging.h"
#include "gtest/gtest.h"

#include <atomic>
#include <cmath>
#include <thread>

#include "packages/core/test/common.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/unity_plugins/unity_telemetry_publisher/include/unity_telemetry_publisher.h"

using namespace unity_plugins;

namespace {
constexpr auto s_topic = "telemetry";
constexpr int s_lingerPeriodInMilliseconds = 1000;
constexpr int s_highWaterMark = 1;
}

class UnityTelemetryPublisherTest : public testing::Test {
protected:
    UnityTelemetryPublisherTest() {}

    void SetUp() {}
};

void testUnityTelemetryPublisher(const calibration::CoordinateTransformation& pose, const std::string& address, std::atomic_bool& stop) {

    UnityTelemetryPublisher publisher(address, s_highWaterMark, s_lingerPeriodInMilliseconds);

    while (stop == false) {
        publisher.sendGroundTruthVehiclePose(pose);
    }
};

TEST_F(UnityTelemetryPublisherTest, received_vehicle_pose_telemetry_correctness) {

    std::atomic_bool stop{ false };

    float x = 1.f;
    float y = 1.f;
    float z = 1.f;
    float rodrigues_x = M_PI / 2;
    float rodrigues_y = 0;
    float rodrigues_z = 0;
    calibration::CoordinateTransformation transformation;
    transformation.set_translationx(x);
    transformation.set_translationy(y);
    transformation.set_translationz(z);
    transformation.set_rodriguesrotationx(rodrigues_x);
    transformation.set_rodriguesrotationy(rodrigues_y);
    transformation.set_rodriguesrotationz(rodrigues_z);

    core::test::UniquePortProvider provider;
    std::string address = "tcp://127.0.0.1:" + std::to_string(provider.next_port());
    std::thread thread(testUnityTelemetryPublisher, transformation, address, std::ref(stop));

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<unity_plugins::UnityTelemetryEnvelope> subscriber(context, address, s_topic, s_highWaterMark);

    EXPECT_TRUE(subscriber.poll(std::chrono::milliseconds(1000)));

    unity_plugins::UnityTelemetryEnvelope envelope;
    EXPECT_TRUE(subscriber.recv(envelope));
    EXPECT_TRUE(envelope.has_vehiclepose());
    EXPECT_EQ(envelope.vehiclepose().transformations().translationx(), x);
    EXPECT_EQ(envelope.vehiclepose().transformations().translationy(), y);
    EXPECT_EQ(envelope.vehiclepose().transformations().translationz(), z);
    EXPECT_EQ(envelope.vehiclepose().transformations().rodriguesrotationx(), rodrigues_x);
    EXPECT_EQ(envelope.vehiclepose().transformations().rodriguesrotationy(), rodrigues_y);
    EXPECT_EQ(envelope.vehiclepose().transformations().rodriguesrotationz(), rodrigues_z);

    stop = true;
    thread.join();
}
