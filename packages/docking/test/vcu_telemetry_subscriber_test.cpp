//
// Created by byungsookim on 5/28/17.
//

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/core/test/common.h"
#include "packages/docking/vcu_telemetry_subscriber.h"
#include "synthetic_vcu_telemetry_publisher.h"

using core::test::UniquePortProvider;

class VcuTelemetrySubscriberTest : public testing::Test {
protected:
    VcuTelemetrySubscriberTest()
        : m_topic("ik_vcu_telemetry")
        , m_highWaterMark(1)
        , m_maxLatencyInSec(1)
        , m_lingerPeriodInMilliseconds(1000) {
        UniquePortProvider provider;
        m_portNumber = provider.next_port();
        m_addr = "tcp://localhost:" + std::to_string(m_portNumber);
    }

    void SetUp() {}

    std::string m_addr;
    int m_portNumber;
    const std::string m_topic;
    const int m_highWaterMark;
    const double m_maxLatencyInSec;
    const int m_lingerPeriodInMilliseconds;
};

TEST_F(VcuTelemetrySubscriberTest, constructor_destructor) { docking::VcuTelemetrySubscriber subscriber(m_addr, m_topic); }

TEST_F(VcuTelemetrySubscriberTest, received_message_correctness) {
    const std::string publishAddr("tcp://*:" + std::to_string(m_portNumber));
    docking::SyntheticVcuTelemetryPublisher publisher(publishAddr, m_highWaterMark, m_lingerPeriodInMilliseconds, m_topic);

    docking::VcuTelemetrySubscriber subscriber(m_addr, m_topic);

    docking::VcuIkTelemetryState telemetryState;
    bool success = subscriber.readTelemetry(telemetryState, 10 * 1e6);

    EXPECT_TRUE(success);
    EXPECT_NEAR(telemetryState.leftServoAngle, M_PI / 2, 1e-4);
}
