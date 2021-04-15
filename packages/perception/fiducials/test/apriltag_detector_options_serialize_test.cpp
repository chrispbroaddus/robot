#include "packages/perception/fiducials/proto/apriltag_detector_options.pb.h"
#include "src/google/protobuf/text_format.h"

#include <iostream>

///
/// Test program to see how the perception::AprilTagConfig looks like in TextFormat
///
int main() {
    perception::AprilTagDetectorOptions options;

    options.set_debug(false);
    options.set_nthreads(1);
    options.set_quaddecimate(4.0);
    options.set_quadsigma(0.0);
    options.set_refinedecode(0);
    options.set_refineedges(0);
    options.set_refinepose(0);

    std::string output;
    google::protobuf::TextFormat::PrintToString(options, &output);

    std::cout << output << std::endl;
}
