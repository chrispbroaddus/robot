#include "packages/docking/proto/inverse_kinematic_docker_options.pb.h"
#include "src/google/protobuf/text_format.h"

#include <iostream>

///
/// Test program to see how the perception::AprilTagConfig looks like in TextFormat
///
int main() {
    docking::InverseKinematicDockerOptions options;

    options.set_telemetry_subscriber_address("tcp://localhost:7001");
    options.set_apriltag_pose_subscriber_address("tcp://localhost:7601");
    options.set_camera_sample_subscriber_address("config/00000000-0000-0000-0000-000000000001/system_calibration.json");
    options.set_docking_status_publisher_address("tcp://*:5951");
    options.set_station_finder_publisher_port("5961");
    options.set_command_client_address("tcp://localhost:5803");
    options.set_stop_criteria_remaining_distance(0.5);
    options.set_velocity_alpha(1.0);
    options.set_use_ground_truth_fiducials(false);
    options.set_april_tag_side_length(0.24);
    options.set_last_command_period_in_milliseconds(100);

    std::string output;
    google::protobuf::TextFormat::PrintToString(options, &output);

    std::cout << output << std::endl;
}