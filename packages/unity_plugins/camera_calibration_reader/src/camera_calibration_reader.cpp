#include "packages/unity_plugins/camera_calibration_reader/include/camera_calibration_reader.h"
#include "glog/logging.h"
#include "google/protobuf/util/json_util.h"
#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/hal/utilities/coordinate_frame_id_utils.h"
#include <exception>
#include <fstream>
#include <sstream>

using namespace unity_plugins;

CameraCalibrationReader::CameraCalibrationReader(const std::istream& jsonStream) {

    if (!jsonStream) {
        throw std::runtime_error("The json stream data is invalid.");
    }

    calibration::SystemCalibration systemCalibration;

    std::stringstream buffer;
    buffer << jsonStream.rdbuf();
    auto status = google::protobuf::util::JsonStringToMessage(buffer.str(), &systemCalibration);

    if (!status.ok()) {
        throw std::runtime_error("The calibration file contains invalid data: " + status.ToString());
    }

    if (systemCalibration.cameraintrinsiccalibration().size() == 0) {
        throw std::runtime_error("There is no intrinsic calibration data.");
    }

    for (int i = 0; i < systemCalibration.cameraintrinsiccalibration().size(); i++) {

        const auto& calibration = systemCalibration.cameraintrinsiccalibration(i);

        const auto& name = calibration.cameraundercalibration().name();

        hal::CameraId cameraId;
        if (!CameraId_Parse(name, &cameraId)) {
            throw std::runtime_error("Invalid calibration json file. Invalid cameraId: " + name);
        }

        CameraCalibrationToMarshal calibrationToMarshal;

        try {
            calibrationToMarshal.resolutionX = calibration.resolutionx();
            calibrationToMarshal.resolutionY = calibration.resolutiony();
            calibrationToMarshal.focalLengthX = calibration.scaledfocallengthx();
            calibrationToMarshal.focalLengthY = calibration.scaledfocallengthy();
            calibrationToMarshal.centerX = calibration.opticalcenterx();
            calibrationToMarshal.centerY = calibration.opticalcentery();

            calibrationToMarshal.kb4_0 = calibration.kannalabrandt().radialdistortioncoefficientk(0);
            calibrationToMarshal.kb4_1 = calibration.kannalabrandt().radialdistortioncoefficientk(1);
            calibrationToMarshal.kb4_2 = calibration.kannalabrandt().radialdistortioncoefficientk(2);
            calibrationToMarshal.kb4_3 = calibration.kannalabrandt().radialdistortioncoefficientk(3);
        } catch (const std::exception&) {
            throw std::runtime_error("Invalid calibration json file.");
        }

        m_camera_calibrations[cameraId] = calibrationToMarshal;
    }

    for (int i = 0; i < systemCalibration.devicetodevicecoordinatetransformation().size(); i++) {

        const auto& extrinsic = systemCalibration.devicetodevicecoordinatetransformation(i);

        source_target_frame_pair_t frame_pair;

        const auto& sourceFrameName = extrinsic.sourcecoordinateframe().device().name();
        frame_pair.first = hal::getCoordinateFrameIdFromDeviceName(sourceFrameName);

        const auto& targetFrameName = extrinsic.targetcoordinateframe().device().name();
        frame_pair.second = hal::getCoordinateFrameIdFromDeviceName(targetFrameName);

        m_extrinsic_calibrations[frame_pair].rodriguesx = extrinsic.rodriguesrotationx();
        m_extrinsic_calibrations[frame_pair].rodriguesy = extrinsic.rodriguesrotationy();
        m_extrinsic_calibrations[frame_pair].rodriguesz = extrinsic.rodriguesrotationz();

        m_extrinsic_calibrations[frame_pair].translationx = extrinsic.translationx();
        m_extrinsic_calibrations[frame_pair].translationy = extrinsic.translationy();
        m_extrinsic_calibrations[frame_pair].translationz = extrinsic.translationz();
    }
}

CameraCalibrationReader::CameraCalibrationReader(const std::string& filename)
    : CameraCalibrationReader(std::ifstream(filename)) {}

CameraCalibrationToMarshal CameraCalibrationReader::get(const hal::CameraId cameraId) { return m_camera_calibrations[cameraId]; }
ExtrinsicCalibrationToMarshal CameraCalibrationReader::get(
    const hal::CoordinateFrameId sourceFrameId, const hal::CoordinateFrameId targetFrameId) {
    auto key = source_target_frame_pair_t(sourceFrameId, targetFrameId);
    if (m_extrinsic_calibrations.find(key) == m_extrinsic_calibrations.end()) {
        throw std::runtime_error("The requested extrinsic information is missing, source: " + std::to_string(sourceFrameId) + ", target : "
            + std::to_string(targetFrameId));
    }
    return m_extrinsic_calibrations[key];
}
