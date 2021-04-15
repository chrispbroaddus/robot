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

TEST_F(VcuTelemetryPublisherTest, constructor_destructor) {
    VcuTelemetryPublisher publisher(s_address, s_highWaterMark, s_lingerPeriodInMilliseconds, s_wheelsRadius);
}
