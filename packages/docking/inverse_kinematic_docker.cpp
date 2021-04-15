#include "inverse_kinematic_docker.h"
#include "apriltag_fiducial_pose_source.h"
#include "ground_truth_fiducial_pose_source.h"
#include "packages/math/geometry/quaternion.h"
#include "packages/planning/vcu_trajectory_check.h"

#include <cmath>

DEFINE_string(inverse_kinematic_docker_options, "", "InverseKinematicDocker-specific options.");
DEFINE_bool(zaxis_slew_control_on, false, "When flag is on, the InverseKinematicDocker sends command to control the z-axis and/or slew.");

namespace docking {

constexpr int HIGH_WATERMARK_DEFAULT = 1;
constexpr int LINGER_PERIOD_IN_MILLISECONDS_DEFAULT = 1000;
constexpr uint64_t READ_TIMEWINDOW_IN_NANOS_DEFAULT = 100 * 1e9;

InverseKinematicDockerOptions loadInverseKinematicDockerOptions(const std::string& txtPbFile) {
    InverseKinematicDockerOptions options;
    std::ifstream t(txtPbFile);
    std::stringstream buffer;
    buffer << t.rdbuf();
    CHECK(google::protobuf::TextFormat::MergeFromString(buffer.str(), &options));
    return options;
}

InverseKinematicDockerOptions loadDefaultInverseKinematicDockerOptions() {
    return loadInverseKinematicDockerOptions(kInverseKinematicDockerOptions);
}

InverseKinematicDocker::InverseKinematicDocker(std::shared_ptr<executor::Executor> executor,
    std::shared_ptr<FiducialPoseSourceInterface> fiducialPoseSource, const VehicleCalibration& zippyCalibration,
    const DockingStation& dockingStation, const InverseKinematicDockerOptions& options)
    : m_sequenceNumber(0)
    , m_lastCommandPaddingPeriodInMillisecond(options.last_command_period_in_milliseconds())
    , m_useGroundTruthApriltagPoses(options.use_ground_truth_fiducials())
    , m_telemetrySubscriber(options.telemetry_subscriber_address(), "telemetry")
    , m_fiducialPoseSubscriber(fiducialPoseSource)
    , m_dockingStatusPublisher(new DockingStatusPublisher(options.docking_status_publisher_address(), 1, 1000))
    , m_planner(new InverseKinematicsController(zippyCalibration))
    , m_velocityAlpha(options.velocity_alpha())
    , m_stopDistanceThreshold(options.stop_criteria_remaining_distance())
    , m_numFailedAttempt(0)
    , m_maxNumFailedAttempt(20)
    , m_stopFlag(false)
    , m_context(1)
    , m_client(m_context, options.command_client_address(), 1000, 1000)
    , m_dockingStation(dockingStation)
    , m_executor(executor)
    , m_stop(false)
    , m_thread(&InverseKinematicDocker::update, this, std::ref(m_stop)) {
    LOG(INFO) << __PRETTY_FUNCTION__ << "telemetrySubscriberAddress     : " << options.telemetry_subscriber_address();
    LOG(INFO) << __PRETTY_FUNCTION__ << "apriltagPoseSubscriberAddress  : " << options.apriltag_pose_subscriber_address();
    LOG(INFO) << __PRETTY_FUNCTION__ << "commandPublisherAddress        : " << options.command_client_address();

    if (m_useGroundTruthApriltagPoses) {
        m_fiducialPoseSubscriber.reset(new GroundTruthFiducialPoseSource(options.apriltag_pose_subscriber_address(), "fiducial_poses"));
    }
}

void InverseKinematicDocker::update(std::atomic_bool& stop) {

    m_prevRemainingDistance = std::numeric_limits<float>::max();
    {
        std::lock_guard<std::mutex> guard(m_mutexStopFlag);
        m_stopFlag = false;
    }
    while (!stop) {

        m_sequenceNumber++;

        teleop::DockingStatus dockingStatus;
        dockingStatus.set_remaining_distance_x(m_planner->remainingDistanceParallelFromTarget());
        dockingStatus.set_remaining_distance_y(m_planner->remainingDistancePerpendicularFromTarget());
        dockingStatus.set_remaining_angle(m_planner->remainingAngle());
        dockingStatus.set_status(teleop::DockingStatus::INPROGRESS);

        m_dockingStatusPublisher->publish(dockingStatus);

        float remainingDistance
            = std::sqrt(std::pow(dockingStatus.remaining_distance_x(), 2) + std::pow(dockingStatus.remaining_distance_y(), 2));
        VLOG(1) << __PRETTY_FUNCTION__ << " ... remaining distance : " << remainingDistance;

        if (remainingDistance < m_stopDistanceThreshold) {
            std::lock_guard<std::mutex> guard(m_mutexStopFlag);
            m_stopFlag = true;
            dockingStatus.set_status(teleop::DockingStatus::SUCCESS);
            m_dockingStatusPublisher->publish(dockingStatus);
            LOG(INFO) << __PRETTY_FUNCTION__ << " ... docking succeeded.";
            break;
        }
        m_prevRemainingDistance = remainingDistance;

        VcuIkTelemetryState telemetryState;

        if (!FLAGS_zaxis_slew_control_on) {
            auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();
            telemetryState.leftServoAngle = 0;
            telemetryState.timestampLeftServoAngle = gpsTimestamp.count();
            telemetryState.rightServoAngle = 0;
            telemetryState.timestampRightServoAngle = gpsTimestamp.count();
            telemetryState.leftServoHeight = 0;
            telemetryState.timestampLeftServoHeight = gpsTimestamp.count();
            telemetryState.rightServoHeight = 0;
            telemetryState.timestampRightServoHeight = gpsTimestamp.count();
        } else if (!m_telemetrySubscriber.readTelemetry(telemetryState, READ_TIMEWINDOW_IN_NANOS_DEFAULT)) {
            continue;
        }

        std::vector<calibration::CoordinateTransformation> posesCameraWrtFiducial;
        if (!m_fiducialPoseSubscriber || !m_fiducialPoseSubscriber->readPoses(posesCameraWrtFiducial, m_dockingStation)) {
            LOG(WARNING) << __PRETTY_FUNCTION__ << " ... fiducial poses are not available.";
            continue;
        }
        CHECK(posesCameraWrtFiducial.size() > 0);

        // Generate VCU command envelope.
        hal::VCUCommandEnvelope envelope;
        envelope.set_sequencenumber(m_sequenceNumber);

        InverseKinematicsController::RawVCUIKControlCommand rawInverseKinematicCommand;
        computeInverseKinematicCommand(rawInverseKinematicCommand, posesCameraWrtFiducial, telemetryState);

        if (rawInverseKinematicCommand.delta_wheel_center_z_in_meter <= 0) {
            LOG(WARNING) << __PRETTY_FUNCTION__ << " ... rawInverseKinematicCommand.delta_wheel_center_z_in_meter cannot be negative: "
                         << rawInverseKinematicCommand.delta_wheel_center_z_in_meter;
            continue;
        }

        core::Point3d plannerTargetPoint;

        // Threshold the target point at 5 meter front.
        float w = std::min(5 / std::abs(rawInverseKinematicCommand.delta_wheel_center_z_in_meter), 1.f);
        plannerTargetPoint.set_x(w * rawInverseKinematicCommand.delta_wheel_center_z_in_meter);
        plannerTargetPoint.set_y(w * rawInverseKinematicCommand.delta_wheel_center_x_in_meter);
        plannerTargetPoint.set_z(0);

        planning::Trajectory pTrajectory;
        m_executor->planTrajectory(plannerTargetPoint, pTrajectory); // TODO: AUTO-440 : resolve SRP violation

        if (m_prevRemainingDistance > m_maxDistanceToControlSlew && pTrajectory.elements_size() == 0) {
            m_numFailedAttempt++;
            LOG(INFO) << __PRETTY_FUNCTION__ << " ... rejected by planner, # of trial : " << m_numFailedAttempt;
            if (m_numFailedAttempt >= m_maxNumFailedAttempt) {
                std::lock_guard<std::mutex> guard(m_mutexStopFlag);
                m_stopFlag = true;
                dockingStatus.set_status(teleop::DockingStatus::FAILURE);
                LOG(INFO) << __PRETTY_FUNCTION__ << " ... docking failed.";
                break;
            }
        }
        m_numFailedAttempt = 0;

        auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

        if (!FLAGS_zaxis_slew_control_on) {
            if (pTrajectory.elements_size() == 0) {
                LOG(WARNING) << __PRETTY_FUNCTION__ << " ... Not executing any command since FLAGS_zaxis_slew_control_on is off "
                             << "and planner rejected to plan the path from the raw IK target point.";
                continue;
            }
            hal::VCUTrajectoryCommand* vcuTrajectoryCommand = new hal::VCUTrajectoryCommand();
            CHECK_NOTNULL(vcuTrajectoryCommand);

            for (int i = 0; i < pTrajectory.elements_size(); ++i) {
                hal::VCUArcDriveSegment arcDriveSegment;
                arcDriveSegment.set_curvatureinversemeters(pTrajectory.elements(i).curvature());
                arcDriveSegment.set_linearvelocitymeterspersecond(m_velocityAlpha * pTrajectory.elements(i).linear_velocity());
                auto segment = vcuTrajectoryCommand->add_segments();
                *segment->mutable_arcdrive() = arcDriveSegment;

                auto timeDiff = pTrajectory.elements(i).relative_time().nanos() - pTrajectory.elements(0).relative_time().nanos();
                CHECK(i == 0 || timeDiff > 0);

                VLOG(2) << " ... ... segment, timediff : " << timeDiff;

                core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
                CHECK_NOTNULL(systemTimestamp);

                systemTimestamp->set_nanos(gpsTimestamp.count() + timeDiff + 10 * 1e6);
                segment->set_allocated_targetstarttime(systemTimestamp);
            }

            hal::VCUArcDriveSegment arcDriveSegment;
            arcDriveSegment.set_curvatureinversemeters(vcuTrajectoryCommand->segments(0).arcdrive().curvatureinversemeters());
            arcDriveSegment.set_linearvelocitymeterspersecond(0);
            auto segment = vcuTrajectoryCommand->add_segments();
            *segment->mutable_arcdrive() = arcDriveSegment;

            core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
            CHECK_NOTNULL(systemTimestamp);

            // let the vehicle stop moving in 100 ms, unless the command is overwritten
            systemTimestamp->set_nanos(
                vcuTrajectoryCommand->segments(0).targetstarttime().nanos() + m_lastCommandPaddingPeriodInMillisecond * 1e6);
            segment->set_allocated_targetstarttime(systemTimestamp);

            envelope.set_allocated_trajectorycommand(vcuTrajectoryCommand);
        } else // if FLAGS_zaxis_slew_control_on == TRUE
        {
            hal::VCUIKControlCommand* vcuikControlCommand = new hal::VCUIKControlCommand();
            CHECK_NOTNULL(vcuikControlCommand);

            // control wheels and pitch separately, mostly for the safety issue.
            if (pTrajectory.elements_size() > 0) {
                for (int i = 0; i < pTrajectory.elements_size(); ++i) {
                    auto segment = vcuikControlCommand->add_segments();
                    if (m_prevRemainingDistance > m_maxDistanceToControlSlew) {
                        segment->set_left_rail_pitch(0);
                        segment->set_right_rail_pitch(0);
                        segment->set_left_rail_elevation(0);
                        segment->set_right_rail_elevation(0);
                    } else {
                        segment->set_left_rail_pitch(rawInverseKinematicCommand.delta_left_rail_servo);
                        segment->set_right_rail_pitch(rawInverseKinematicCommand.delta_right_rail_servo);
                        segment->set_left_rail_elevation(rawInverseKinematicCommand.delta_left_slider_value);
                        segment->set_right_rail_elevation(rawInverseKinematicCommand.delta_right_slider_value);
                    }

                    segment->set_curvatureinversemeters(pTrajectory.elements(i).curvature());
                    segment->set_linearvelocitymeterspersecond(m_velocityAlpha * pTrajectory.elements(i).linear_velocity());

                    auto timeDiff = pTrajectory.elements(i).relative_time().nanos() - pTrajectory.elements(0).relative_time().nanos();
                    CHECK(i == 0 || timeDiff > 0);

                    VLOG(1) << " ... ... segment, timediff : " << timeDiff;

                    core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
                    CHECK_NOTNULL(systemTimestamp);

                    systemTimestamp->set_nanos(gpsTimestamp.count() + timeDiff + 10 * 1e6);
                    segment->set_allocated_target_start_time(systemTimestamp);
                }
                CHECK(pTrajectory.elements_size() == vcuikControlCommand->segments_size());
            }
            // when the distance is close, the absence of planner trajectory is allowed.
            else if (m_prevRemainingDistance <= m_maxDistanceToControlSlew) {
                auto segment = vcuikControlCommand->add_segments();
                segment->set_left_rail_pitch(rawInverseKinematicCommand.delta_left_rail_servo);
                segment->set_right_rail_pitch(rawInverseKinematicCommand.delta_right_rail_servo);
                segment->set_left_rail_elevation(rawInverseKinematicCommand.delta_left_slider_value);
                segment->set_right_rail_elevation(rawInverseKinematicCommand.delta_right_slider_value);
                segment->set_curvatureinversemeters(0);
                segment->set_linearvelocitymeterspersecond(0);

                core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
                CHECK_NOTNULL(systemTimestamp);
                systemTimestamp->set_nanos(gpsTimestamp.count() + 10 * 1e6);
                segment->set_allocated_target_start_time(systemTimestamp);
            }

            auto segment = vcuikControlCommand->add_segments();
            segment->set_left_rail_pitch(0);
            segment->set_right_rail_pitch(0);
            segment->set_left_rail_elevation(0);
            segment->set_right_rail_elevation(0);
            segment->set_curvatureinversemeters(vcuikControlCommand->segments(0).curvatureinversemeters());
            segment->set_linearvelocitymeterspersecond(0);

            core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
            CHECK_NOTNULL(systemTimestamp);

            // let the vehicle stop moving in 100 ms, unless the command is overwritten
            systemTimestamp->set_nanos(
                vcuikControlCommand->segments(0).target_start_time().nanos() + m_lastCommandPaddingPeriodInMillisecond * 1e6);
            segment->set_allocated_target_start_time(systemTimestamp);

            envelope.set_allocated_ikcontrolcommand(vcuikControlCommand);
        }

        CHECK(m_client.send(envelope));

        int errorReason;
        planning::commandValidate(envelope, errorReason);
        LOG(INFO) << __PRETTY_FUNCTION__ << " ... command_validate error reason : " << errorReason;

        hal::VCUCommandResponse response;
        m_client.recv(response);
        LOG(INFO) << __PRETTY_FUNCTION__ << " ... VCUCommandResponse error reason : " << response.error_reason();
    }
}

void InverseKinematicDocker::computeInverseKinematicCommand(InverseKinematicsController::RawVCUIKControlCommand& rawCommand,
    const std::vector<calibration::CoordinateTransformation>& poses, const VcuIkTelemetryState& telemetryState) {

    Sophus::SE3f se3CameraWrtTarget;

    VLOG(2) << __PRETTY_FUNCTION__ << " ... posesCameraWrtFiducial.size() : " << poses.size();

    for (size_t i = 0; i < poses.size(); i++) {
        const auto& pose = poses[i];

        Sophus::SE3f se3CameraWrtFiducial;

        se3CameraWrtFiducial.translation() = Eigen::Matrix<float, 3, 1>(pose.translationx(), pose.translationy(), pose.translationz());
        se3CameraWrtFiducial.setQuaternion(
            geometry::convertRodriguesToQuaternion<float>(pose.rodriguesrotationx(), pose.rodriguesrotationy(), pose.rodriguesrotationz()));
        Sophus::SE3f se3TargetWrtFiducial;
        for (int j = 0; j < m_dockingStation.fiducials_size(); j++) {
            const auto& stationFiducial = m_dockingStation.fiducials(j);
            if (pose.sourcecoordinateframe().device().name() == stationFiducial.transformation().sourcecoordinateframe().device().name()) {
                se3TargetWrtFiducial.translation() = Eigen::Matrix<float, 3, 1>(stationFiducial.transformation().translationx(),
                    stationFiducial.transformation().translationy(), stationFiducial.transformation().translationz());
                se3TargetWrtFiducial.setQuaternion(
                    geometry::convertRodriguesToQuaternion<float>(stationFiducial.transformation().rodriguesrotationx(),
                        stationFiducial.transformation().rodriguesrotationy(), stationFiducial.transformation().rodriguesrotationz()));
            }
        }

        // TODO: AUTO-268 : Use multi-apriltags to estimate MLE solution for the target location
        Sophus::SE3f se3FiducialWrtTarget = se3TargetWrtFiducial.inverse();
        VLOG(2) << __PRETTY_FUNCTION__ << " ... ... se3FiducialWrtTarget: ";
        VLOG(2) << __PRETTY_FUNCTION__ << " ... ... ... q() : " << se3FiducialWrtTarget.unit_quaternion().coeffs();
        VLOG(2) << __PRETTY_FUNCTION__ << " ... ... ... t() : " << se3FiducialWrtTarget.translation();

        se3CameraWrtTarget = se3CameraWrtFiducial * se3FiducialWrtTarget;
        VLOG(2) << __PRETTY_FUNCTION__ << " ... ... se3CameraWrtTarget: ";
        VLOG(2) << __PRETTY_FUNCTION__ << " ... ... ... q() : " << se3CameraWrtTarget.unit_quaternion().coeffs();
        VLOG(2) << __PRETTY_FUNCTION__ << " ... ... ... t() : " << se3CameraWrtTarget.translation();
    }

    VLOG(2) << __PRETTY_FUNCTION__ << " ... ... telemetry.leftServoHeight: " << telemetryState.leftServoHeight;
    VLOG(2) << __PRETTY_FUNCTION__ << " ... ... telemetry.rightServoHeight: " << telemetryState.rightServoHeight;
    VLOG(2) << __PRETTY_FUNCTION__ << " ... ... telemetry.leftServoAngle: " << telemetryState.leftServoAngle;
    VLOG(2) << __PRETTY_FUNCTION__ << " ... ... telemetry.rightServoAngle: " << telemetryState.rightServoAngle;

    m_planner->planTo(rawCommand, se3CameraWrtTarget, telemetryState);

    VLOG(2) << __PRETTY_FUNCTION__ << " ... ... rawCommand.delta_left_slider_value: " << rawCommand.delta_left_slider_value;
    VLOG(2) << __PRETTY_FUNCTION__ << " ... ... rawCommand.delta_right_slider_value: " << rawCommand.delta_right_slider_value;
    VLOG(2) << __PRETTY_FUNCTION__ << " ... ... rawCommand.delta_left_rail_servo: " << rawCommand.delta_left_rail_servo;
    VLOG(2) << __PRETTY_FUNCTION__ << " ... ... rawCommand.delta_right_rail_servo: " << rawCommand.delta_right_rail_servo;

    VLOG(1) << __PRETTY_FUNCTION__
            << " ... ... rawCommand.delta_wheel_center_z_in_meter (front) : " << rawCommand.delta_wheel_center_z_in_meter;
    VLOG(1) << __PRETTY_FUNCTION__
            << " ... ... rawCommand.delta_wheel_center_x_in_meter (right) : " << rawCommand.delta_wheel_center_x_in_meter;
}

} // docking_manager
