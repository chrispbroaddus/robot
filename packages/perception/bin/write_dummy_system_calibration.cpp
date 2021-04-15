#include <fstream>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/util/json_util.h"

#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/hal/proto/device.pb.h"
#include "packages/perception/utils.h"

DEFINE_string(mode, "", "Mode to generate (dummy, 4camera)");

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);
    gflags::SetUsageMessage("Configuration file generator");
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    FLAGS_alsologtostderr = 1;

    CHECK(!FLAGS_mode.empty()) << "Require -mode to be set";

    std::ofstream output;
    calibration::SystemCalibration calibration;

    if ("dummy" == FLAGS_mode) {
        using perception::generateDummySystemCalibration;
        calibration = generateDummySystemCalibration();
        output.open("DummySystemCalibration.json");
    } else if ("4camera" == FLAGS_mode) {
        using perception::generateBasic4CameraSystemCalibration;
        calibration = generateBasic4CameraSystemCalibration();
        output.open("FourCameraSystemCalibration.json");
    }
    CHECK(output.good());
    std::string jsonString;
    google::protobuf::util::MessageToJsonString(calibration, &jsonString);
    output << jsonString;
    output.close();
    return EXIT_SUCCESS;
}
