#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/core/test/common.h"
#include "packages/docking/inverse_kinematic_docker.h"
#include "packages/docking/proto/docking_station.pb.h"

using core::test::UniquePortProvider;

/// Testing ctor and dtor.
/// docking::InverseKinematicDocker is pipelining class, and
/// more functionality tests are manually done by Unity
TEST(InverseKinematicDockerTest, ctor_and_dtor) {

    UniquePortProvider provider;

    calibration::CameraIntrinsicCalibration cameraIntrinsicCalibration;
    docking::VehicleCalibration vehicleCalibration;
    docking::DockingStation dockingStation;
    std::shared_ptr<executor::Executor> executor;

    std::shared_ptr<docking::FiducialPoseSourceInterface> fiducialPoseSource;
    {
        perception::AprilTagConfig aprilTagConfig = perception::loadDefaultAprilTagConfig();
        perception::AprilTagDetectorOptions aprilTagDetectorOptions = perception::loadDefaultAprilTagDetectorOptions();

        calibration::CameraIntrinsicCalibration calibration;

        fiducialPoseSource.reset(
            new docking::ApriltagFiducialPoseSource("", "camera", calibration, aprilTagConfig, aprilTagDetectorOptions));
    }

    docking::InverseKinematicDockerOptions options;
    options.set_telemetry_subscriber_address("tcp://localhost:" + std::to_string(provider.next_port()));
    options.set_apriltag_pose_subscriber_address("tcp://localhost:" + std::to_string(provider.next_port()));
    options.set_camera_sample_subscriber_address("tcp://localhost:" + std::to_string(provider.next_port()));
    options.set_docking_status_publisher_address("tcp://*:" + std::to_string(provider.next_port()));
    options.set_command_client_address("tcp://localhost:" + std::to_string(provider.next_port()));
    options.set_stop_criteria_remaining_distance(0.5);
    options.set_velocity_alpha(0.7);
    options.set_use_ground_truth_fiducials(false);
    options.set_april_tag_side_length(0.24);
    options.set_last_command_period_in_milliseconds(100);

    docking::InverseKinematicDocker docker(executor, fiducialPoseSource, vehicleCalibration, dockingStation, options);
}
