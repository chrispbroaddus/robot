#pragma once

#include "docking_status_publisher.h"
#include "fiducial_pose_source_interface.h"
#include "fiducial_station_finder.h"
#include "gflags/gflags.h"
#include "inverse_kinematic_controller.h"
#include "packages/core/proto/geometry.pb.h"
#include "packages/core/proto/timestamp.pb.h"
#include "packages/docking/proto/docking_station.pb.h"
#include "packages/docking/proto/inverse_kinematic_docker_options.pb.h"
#include "packages/executor/executor.h"
#include "packages/hal/proto/vcu_command_response.pb.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/net/include/zmq_rep_server.h"
#include "packages/net/include/zmq_req_client.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/perception/fiducials/proto/fiducial_poses.pb.h"
#include "packages/perception/perception.h"
#include "packages/teleop/proto/backend_message.pb.h"
#include "vcu_telemetry_subscriber.h"

#include <mutex>
#include <thread>

DECLARE_string(inverse_kinematic_docker_options);
DECLARE_bool(zaxis_slew_control_on);

namespace docking {

constexpr char kInverseKinematicDockerOptions[] = "config/global/inverse_kinematic_docker_options.default.pbtxt";

InverseKinematicDockerOptions loadInverseKinematicDockerOptions(const std::string& txtPbFile);
InverseKinematicDockerOptions loadDefaultInverseKinematicDockerOptions();

///
/// @brief Inverse-Kinematic Docker is the robot controller, docking the vehicle to the target station
///        based on the AprilTag observations and vehicle telemetry data
///
class InverseKinematicDocker {
public:
    InverseKinematicDocker(std::shared_ptr<executor::Executor> executor, std::shared_ptr<FiducialPoseSourceInterface> m_fiducialPoseSource,
        const VehicleCalibration& zippyCalibration, const DockingStation& dockingStation, const InverseKinematicDockerOptions& options);

    InverseKinematicDocker(const InverseKinematicDocker&) = delete;
    InverseKinematicDocker& operator=(const InverseKinematicDocker&) = delete;
    InverseKinematicDocker& operator=(const InverseKinematicDocker&&) = delete;

    ~InverseKinematicDocker() {
        m_stop = true;
        m_thread.join();
    }

    bool isRunning() {
        std::lock_guard<std::mutex> guard(m_mutexStopFlag);
        return !m_stopFlag;
    }

private:
    ///
    /// @param Infinite loop updating docking command until docking process is terminated
    ///
    void update(std::atomic_bool& stop);

    /// @brief Compute the inverse kinematic command from telemetry and extrinsic data
    /// @params[in] poses : poses from fiducial to camera, or camera w.r.t. fiducial.
    void computeInverseKinematicCommand(InverseKinematicsController::RawVCUIKControlCommand& rawCommand,
        const std::vector<calibration::CoordinateTransformation>& poses, const VcuIkTelemetryState& telemetryState);

    uint64_t m_sequenceNumber;

    int m_lastCommandPaddingPeriodInMillisecond;

    bool m_useGroundTruthApriltagPoses;
    VcuTelemetrySubscriber m_telemetrySubscriber;
    std::shared_ptr<FiducialPoseSourceInterface> m_fiducialPoseSubscriber;
    std::unique_ptr<DockingStatusPublisher> m_dockingStatusPublisher;
    std::unique_ptr<InverseKinematicsController> m_planner;

    float m_velocityAlpha;
    float m_prevRemainingDistance;
    float m_maxDistanceToControlSlew;

    // parameters to stop docking
    float m_stopDistanceThreshold;
    int m_numFailedAttempt;
    int m_maxNumFailedAttempt;
    std::mutex m_mutexStopFlag;
    bool m_stopFlag;

    zmq::context_t m_context;
    net::ZMQProtobufReqClient<hal::VCUCommandEnvelope, hal::VCUCommandResponse> m_client;
    std::mutex m_guard;
    DockingStation m_dockingStation;
    std::shared_ptr<executor::Executor> m_executor;
    std::atomic_bool m_stop;
    std::thread m_thread;
};

} // class InverseKinematicDocker
