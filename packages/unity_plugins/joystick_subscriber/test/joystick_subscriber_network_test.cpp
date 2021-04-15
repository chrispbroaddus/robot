//
// Created by byungsookim on 5/28/17.
//

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/unity_plugins/joystick_subscriber/include/joystick_subscriber.h"
#include "synthetic_joystick_publisher.h"

#include <chrono>
#include <thread>

using namespace unity_plugins;

class JoystickSubscriberTest : public testing::Test {
protected:
    JoystickSubscriberTest()
        : m_addr("tcp://127.0.0.1:6000")
        , m_highWaterMark(1)
        , m_axisSize(2)
        , m_maxLatencyInSec(1)
        , m_lingerPeriodInMilliseconds(1000) {}

    void SetUp() {}

    const std::string m_addr;
    const int m_highWaterMark;
    const int m_axisSize;
    const double m_maxLatencyInSec;
    const int m_lingerPeriodInMilliseconds;
};

TEST_F(JoystickSubscriberTest, received_message_correctness) {
    SyntheticJoystickPublisher publisher(m_addr, m_highWaterMark, m_lingerPeriodInMilliseconds);

    JoystickSubscriber subscriber(m_addr, m_highWaterMark, m_axisSize, m_maxLatencyInSec);

    hal::JoystickSample sample;
    bool success = subscriber.receive(sample);
    EXPECT_TRUE(success);
    EXPECT_EQ(sample.axis(0), -1);
    EXPECT_EQ(sample.axis(1), +1);
}
