#include "packages/perception/fiducials/proto/apriltag_config.pb.h"
#include "src/google/protobuf/text_format.h"

#include <iostream>

///
/// Test program to see how the perception::AprilTagConfig looks like in TextFormat
///
int main() {
    perception::AprilTagConfig config;
    config.set_apriltagfamily(perception::AprilTagFamily::AprilTag36h11);
    config.set_sidelengthinmeters(0.24);
    config.set_border(1);

    std::string output;
    google::protobuf::TextFormat::PrintToString(config, &output);

    std::cout << output << std::endl;
}
