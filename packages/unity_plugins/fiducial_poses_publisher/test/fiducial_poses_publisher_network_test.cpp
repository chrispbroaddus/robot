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

constexpr char s_topic[]{ "fiducial_poses" };
constexpr int s_lingerPeriodInMilliseconds{ 6000 };
constexpr int s_highWaterMark{ 1 };

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

void testPublisher(const FiducialPosesPublisher::FiducialPose& pose, int portNumber, std::atomic_bool& s_stop) {

    FiducialPosesPublisher publisher("tcp://127.0.0.1:" + std::to_string(portNumber), s_highWaterMark, s_lingerPeriodInMilliseconds);
    const auto targetCoordinate = FiducialPosesPublisher::TargetCoordinate::FRONT_FISHEYE_CAM;

    publisher.add(targetCoordinate, "test_marker", 0, pose);

    while (s_stop == false) {
        publisher.send();
    }
};

TEST_F(FiducialPosesPublisherTest, received_data_correctness) {

    std::atomic_bool stop{ false };

    FiducialPosesPublisher::FiducialPose fiducialPoseInUnity;
    fiducialPoseInUnity.rodrigues_x = -M_PI / 2;
    fiducialPoseInUnity.rodrigues_y = M_PI / 4;
    fiducialPoseInUnity.rodrigues_z = -M_PI / 3;
    fiducialPoseInUnity.translation_x = 1;
    fiducialPoseInUnity.translation_y = 2;
    fiducialPoseInUnity.translation_z = -1;

    float rxp, ryp, rzp, txp, typ, tzp;
    convertUnityAprilTagToDockingAprilTagCoordinate(rxp, ryp, rzp, txp, typ, tzp, fiducialPoseInUnity.rodrigues_x,
        fiducialPoseInUnity.rodrigues_y, fiducialPoseInUnity.rodrigues_z, fiducialPoseInUnity.translation_x,
        fiducialPoseInUnity.translation_y, fiducialPoseInUnity.translation_z);

    std::thread thread(testPublisher, fiducialPoseInUnity, m_portNumber, std::ref(stop));

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<perception::FiducialPoses> subscriber(
        context, "tcp://127.0.0.1:" + std::to_string(m_portNumber), s_topic, s_highWaterMark);

    EXPECT_TRUE(subscriber.poll(std::chrono::milliseconds(1000)));

    perception::FiducialPoses rPoses;
    subscriber.recv(rPoses);
    EXPECT_EQ(rPoses.transformations_size(), 1);
    EXPECT_EQ(rPoses.transformations(0).rodriguesrotationx(), rxp);
    EXPECT_EQ(rPoses.transformations(0).rodriguesrotationy(), ryp);
    EXPECT_EQ(rPoses.transformations(0).rodriguesrotationz(), rzp);
    EXPECT_EQ(rPoses.transformations(0).translationx(), txp);
    EXPECT_EQ(rPoses.transformations(0).translationy(), typ);
    EXPECT_EQ(rPoses.transformations(0).translationz(), tzp);

    stop = true;
    thread.join();
}
