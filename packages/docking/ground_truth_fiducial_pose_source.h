#pragma once

#include "fiducial_pose_source_interface.h"
#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "packages/docking/proto/docking_station.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/perception/fiducials/apriltag_detector.h"
#include "packages/perception/fiducials/proto/fiducial_poses.pb.h"
#include <atomic>
#include <functional>
#include <mutex>
#include <thread>

namespace docking {

///
/// Subscribe poses of the ground truth fiducial poses from the unity telemetry.
///
class GroundTruthFiducialPoseSource : public FiducialPoseSourceInterface {
public:
    ///
    /// \param addr    The address to subscribe fiducial poses
    /// \param topic   ZMQ topic
    ///
    GroundTruthFiducialPoseSource(const std::string& addr, const std::string& topic);

    GroundTruthFiducialPoseSource(const GroundTruthFiducialPoseSource& obj) = delete;
    GroundTruthFiducialPoseSource& operator=(GroundTruthFiducialPoseSource other) = delete;

    ~GroundTruthFiducialPoseSource();

    ///
    /// \brief Read poses for a specific docking station
    /// \param poses    Poses of the associate fiducials
    /// \param dockingStation    The docking station meta data
    /// \return true : if non-outdated the data exist
    bool readPoses(std::vector<calibration::CoordinateTransformation>& poses, const docking::DockingStation& dockingStation) override;

private:
    ///
    /// The function listening fiducial poses, running in a thread when the instance constructed.
    /// \param addr    ZMQ subscription address
    /// \param topic   ZMQ topic
    /// \param stop    atomic bool to control continuing/stopping the thread
    ///
    void listen(const std::string& addr, const std::string& topic, std::atomic_bool& stop);

    /// controller for a thread
    std::atomic_bool m_stop;

    std::unique_ptr<perception::AprilTagDetector> m_detector;

    /// Keep the poses read from the ZMQ subscriber
    perception::FiducialPoses m_poses;
    std::thread m_thread;
    std::mutex m_mutex;
};
}
