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
    UnityTelemetryPublisherTest() { m_portNumber = m_provider.next_port(); }

    void SetUp() {}

    int m_portNumber;
    core::test::UniquePortProvider m_provider;
};

TEST_F(UnityTelemetryPublisherTest, constructor_destructor) {
    std::string address = "tcp://127.0.0.1:" + std::to_string(m_portNumber);
    UnityTelemetryPublisher publisher(address, s_highWaterMark, s_lingerPeriodInMilliseconds);
}
