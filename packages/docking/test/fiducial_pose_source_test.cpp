//
// Created by byungsookim on 5/28/17.
//

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/core/test/common.h"
#include "packages/docking/ground_truth_fiducial_pose_source.h"
#include "synthetic_fiducial_pose_publisher.h"

#include <chrono>
#include <thread>

using core::test::UniquePortProvider;

class FiducialPoseSourceTest : public testing::Test {
protected:
    FiducialPoseSourceTest()
        : m_topic("fiducial_poses")
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

TEST_F(FiducialPoseSourceTest, constructor_destructor) { docking::GroundTruthFiducialPoseSource subscriber(m_addr, m_topic); }

TEST_F(FiducialPoseSourceTest, received_message_correctness) {
    const std::string publishAddr("tcp://*:" + std::to_string(m_portNumber));
    docking::SyntheticFiducialPosePublisher publisher(publishAddr, m_highWaterMark, m_lingerPeriodInMilliseconds, m_topic);

    docking::GroundTruthFiducialPoseSource subscriber(m_addr, m_topic);

    hal::Device* stationAnchorDevice = new hal::Device();
    stationAnchorDevice->set_name("station_anchor");

    calibration::CoordinateFrame* targetFrame = new calibration::CoordinateFrame();
    targetFrame->set_allocated_device(stationAnchorDevice);

    hal::Device* sourceDevice = new hal::Device();
    sourceDevice->set_name("test_fiducial");

    calibration::CoordinateFrame* sourceFrame = new calibration::CoordinateFrame();
    sourceFrame->set_allocated_device(sourceDevice);

    calibration::CoordinateTransformation* transformationAnchorWrtFiducial = new calibration::CoordinateTransformation();
    transformationAnchorWrtFiducial->set_allocated_sourcecoordinateframe(sourceFrame);
    transformationAnchorWrtFiducial->set_allocated_targetcoordinateframe(targetFrame);

    docking::DockingStation station;
    station.set_station_id(0);
    auto fiducial = station.add_fiducials();
    fiducial->set_allocated_transformation(transformationAnchorWrtFiducial);

    std::vector<calibration::CoordinateTransformation> poses;
    bool success = subscriber.readPoses(poses, station);

    EXPECT_TRUE(success);
    EXPECT_TRUE(poses.size() == 1);
}
