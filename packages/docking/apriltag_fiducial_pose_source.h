#pragma once

#include "fiducial_pose_source_interface.h"
#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "packages/docking/proto/docking_station.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/perception/fiducials/apriltag_detector.h"
#include "packages/perception/fiducials/proto/fiducial_poses.pb.h"
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

namespace docking {

///
/// Subscribe image stream and detect fiducials to return poses
///
class ApriltagFiducialPoseSource : public FiducialPoseSourceInterface {
public:
    ///
    /// \param cameraSampleSubscribeAddress    The address to subscribe image stream
    /// \param topic   ZMQ topic
    ///
    ApriltagFiducialPoseSource(const std::string& cameraSampleSubscribeAddress, const std::string& topic,
        const calibration::CameraIntrinsicCalibration& cameraIntrinsicCalibration, const perception::AprilTagConfig& aprilTagConfig,
        const perception::AprilTagDetectorOptions& aprilTagDetectorOptions);

    ApriltagFiducialPoseSource(const ApriltagFiducialPoseSource& obj) = delete;
    ApriltagFiducialPoseSource& operator=(ApriltagFiducialPoseSource other) = delete;

    ~ApriltagFiducialPoseSource();

    ///
    /// \brief Read poses for a specific docking station
    /// \param poses    Poses of the associate fiducials
    /// \param dockingStation    The docking station meta data
    /// \return true : if non-outdated the data exist
    bool readPoses(std::vector<calibration::CoordinateTransformation>& poses, const docking::DockingStation& dockingStation) override;

private:
    ///
    /// The function listening image streams, running in a thread when the instance constructed.
    /// \param addr    ZMQ subscription address
    /// \param topic   ZMQ topic
    /// \param stop    atomic bool to control continuing/stopping the thread
    ///
    void listen(const std::string& addr, const std::string& topic, std::atomic_bool& stop);

    perception::AprilTagDetector m_detector;

    /// controller for a thread
    std::atomic_bool m_stop;

    /// Keep the poses read from the ZMQ subscriber
    hal::CameraSample m_sample;
    std::thread m_thread;
    std::mutex m_mutex;
};
}
