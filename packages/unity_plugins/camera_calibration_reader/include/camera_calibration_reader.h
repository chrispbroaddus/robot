#pragma once

#include "packages/hal/proto/coordinate_frame_id.pb.h"
#include "packages/unity_plugins/utils/include/zippy_image_interop.h"
#include <map>
#include <string>

namespace unity_plugins {

///
/// Subset of calibration parameters being used
///
struct CameraCalibrationToMarshal {
    int resolutionX;
    int resolutionY;
    float focalLengthX;
    float focalLengthY;
    float centerX;
    float centerY;
    float kb4_0;
    float kb4_1;
    float kb4_2;
    float kb4_3;
};

struct ExtrinsicCalibrationToMarshal {
    float rodriguesx;
    float rodriguesy;
    float rodriguesz;
    float translationx;
    float translationy;
    float translationz;
};

///
/// Read Camera Calibration Parameters from ProtoBuf-JSON file,
/// and send (marshal) to unity
///
class CameraCalibrationReader {

public:
    typedef std::pair<hal::CoordinateFrameId, hal::CoordinateFrameId> source_target_frame_pair_t; // first : source, second : target

    CameraCalibrationReader(const std::istream& jsonStream);
    explicit CameraCalibrationReader(const std::string& filename);
    ~CameraCalibrationReader() = default;
    CameraCalibrationReader(const CameraCalibrationReader& obj) = delete;
    CameraCalibrationReader& operator=(CameraCalibrationReader other) = delete;

    ///
    /// \brief Read the calibration data for a specific camera
    /// \param cameraId
    /// \return Calibration parameters
    CameraCalibrationToMarshal get(const hal::CameraId cameraId);
    ExtrinsicCalibrationToMarshal get(const hal::CoordinateFrameId sourceFrameId, const hal::CoordinateFrameId targetFrameId);

private:
    std::map<hal::CameraId, CameraCalibrationToMarshal> m_camera_calibrations;
    std::map<source_target_frame_pair_t, ExtrinsicCalibrationToMarshal> m_extrinsic_calibrations;
};
} // namespace unity_plugins
