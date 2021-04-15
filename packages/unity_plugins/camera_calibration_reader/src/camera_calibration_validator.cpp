#include "glog/logging.h"
#include "packages/unity_plugins/camera_calibration_reader/include/camera_calibration_reader.h"

using namespace unity_plugins;

///
/// @brief Read in a calibration file and check it can be parsed
///
int main(int argc, char** argv) {

    google::InitGoogleLogging(argv[0]);

    if (argc != 2) {
        LOG(ERROR) << "USAGE: " << argv[0] << " CAMERA_CALIBRATION_FILE_NAME";
        return EXIT_FAILURE;
    }

    std::string filename(argv[1]);

    CameraCalibrationReader reader(filename);

    for (int ii = 0; ii < hal::CameraId_ARRAYSIZE; ++ii) {
        auto cameraId = (hal::CameraId)ii;
        auto cameraCalibration = reader.get(cameraId);
    }

    hal::CoordinateFrameId sourceFrameId = hal::CoordinateFrameId::CoordFrameFrontRightStereo;
    hal::CoordinateFrameId targetFrameId = hal::CoordinateFrameId::CoordFrameFrontLeftStereo;
    auto extrinsicCalibration = reader.get(sourceFrameId, targetFrameId);

    sourceFrameId = hal::CoordinateFrameId::CoordFrameBaseLink;

    targetFrameId = hal::CoordinateFrameId::CoordFrameFrontFisheye;
    extrinsicCalibration = reader.get(sourceFrameId, targetFrameId);

    targetFrameId = hal::CoordinateFrameId::CoordFrameRearFisheye;
    extrinsicCalibration = reader.get(sourceFrameId, targetFrameId);

    targetFrameId = hal::CoordinateFrameId::CoordFrameFrontRightStereo;
    extrinsicCalibration = reader.get(sourceFrameId, targetFrameId);

    targetFrameId = hal::CoordinateFrameId::CoordFrameRearLeftStereo;
    extrinsicCalibration = reader.get(sourceFrameId, targetFrameId);

    targetFrameId = hal::CoordinateFrameId::CoordFrameRearRightStereo;
    extrinsicCalibration = reader.get(sourceFrameId, targetFrameId);

    targetFrameId = hal::CoordinateFrameId::CoordFrameLeftFisheye;
    extrinsicCalibration = reader.get(sourceFrameId, targetFrameId);

    targetFrameId = hal::CoordinateFrameId::CoordFrameRightFisheye;
    extrinsicCalibration = reader.get(sourceFrameId, targetFrameId);

    return EXIT_SUCCESS;
}
