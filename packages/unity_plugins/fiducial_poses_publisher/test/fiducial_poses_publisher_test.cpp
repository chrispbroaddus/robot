//
// Created by byungsookim on 5/28/17.
//

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/core/test/common.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/unity_plugins/fiducial_poses_publisher/include/fiducial_poses_publisher.h"
#include "packages/unity_plugins/utils/include/coordinate_conversion.h"

#include <cmath>

using core::test::UniquePortProvider;

UniquePortProvider provider;

using namespace unity_plugins;

namespace {
constexpr auto s_topic = "fiducial_poses";
constexpr int s_lingerPeriodInMilliseconds = 1000;
constexpr int s_highWaterMark = 1;
}

class FiducialPosesPublisherTest : public testing::Test {
protected:
    FiducialPosesPublisherTest()
        : m_portNumber(provider.next_port())
        , m_topic(s_topic)
        , m_highWaterMark(s_highWaterMark)
        , m_lingerPeriodInMilliseconds(s_lingerPeriodInMilliseconds) {}

    void SetUp() {}

    int m_portNumber;
    const std::string m_topic;
    const int m_highWaterMark;
    const int m_lingerPeriodInMilliseconds;
};

TEST_F(FiducialPosesPublisherTest, constructor_destructor) {
    FiducialPosesPublisher publisher("tcp://127.0.0.1:" + std::to_string(m_portNumber), m_highWaterMark, m_lingerPeriodInMilliseconds);
}
