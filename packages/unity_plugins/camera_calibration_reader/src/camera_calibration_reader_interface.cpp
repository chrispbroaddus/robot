#include "packages/unity_plugins/camera_calibration_reader/include/camera_calibration_reader.h"
#include "glog/logging.h"
#include "packages/unity_plugins/utils/include/zippy_interface.h"
#include <memory>

using namespace unity_plugins;

static std::unique_ptr<CameraCalibrationReader> s_reader;

extern "C" {

///
/// \brief Load the calibration data from a single json file to construct CameraCalibrationReader
/// \return 1 : Success, 0 : Fail
///
ZIPPY_INTERFACE_EXPORT int CameraCalibrationReader_Load(char* filename) {
    std::string filenameStr(filename);
    try {
        s_reader.reset(new CameraCalibrationReader(filenameStr));
        return 1;
    } catch (const std::exception&) {
        return 0;
    }
}

///
/// \brief Get the intrinsic calibration parameters for the specified camera
/// \return IntrinsicCalibration parameters
///
ZIPPY_INTERFACE_EXPORT CameraCalibrationToMarshal CameraCalibrationReader_Read(hal::CameraId cameraId) { return s_reader->get(cameraId); }

///
/// \brief Get the calibration parameters for each camera
/// \return ExtrinsicCalibration parameters
///
ZIPPY_INTERFACE_EXPORT ExtrinsicCalibrationToMarshal ExtrinsicCalibrationReader_Read(
    hal::CoordinateFrameId sourceFrameId, hal::CoordinateFrameId targetFrameId) {
    return s_reader->get(sourceFrameId, targetFrameId);
}
}
