#pragma once

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/text_format.h"
#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/perception/fiducials/fiducial_detector_interface.h"
#include "packages/perception/fiducials/proto/apriltag_config.pb.h"
#include "packages/perception/fiducials/proto/apriltag_detector_options.pb.h"
#include <fstream>

#include "apriltag.h"

#include "opencv2/core.hpp"

DECLARE_string(apriltag_config);
DECLARE_string(apriltag_detector_options);

namespace perception {

constexpr char kDefaultAprilTagConfig[] = "config/global/apriltag_config.default.pbtxt";
constexpr char kDefaultAprilTagDetectorOptions[] = "config/global/apriltag_detector_options.default.pbtxt";

AprilTagConfig loadDefaultAprilTagConfig();
AprilTagConfig loadAprilTagConfig(const std::string& textPbText);

AprilTagDetectorOptions loadDefaultAprilTagDetectorOptions();
AprilTagDetectorOptions loadAprilTagDetectorOptions(const std::string& textPbText);

class AprilTagDetector : public FiducialDetectorInterface {
public:
    AprilTagDetector(const calibration::CameraIntrinsicCalibration& cameraIntrinsicCalibration, const AprilTagConfig& aprilTagConfig,
        const AprilTagDetectorOptions& aprilTagDetectorOptions);
    ~AprilTagDetector();
    AprilTagDetector(const AprilTagDetector&) = delete;
    AprilTagDetector(const AprilTagDetector&&) = delete;
    AprilTagDetector& operator=(const AprilTagDetector&) = delete;
    AprilTagDetector& operator=(const AprilTagDetector&&) = delete;

    bool estimatePose(const hal::CameraSample& cameraSample, std::vector<calibration::CoordinateTransformation>& detectedPoses) override;

private:
    cv::Mat m_cameraMatrix;
    cv::Mat m_distortion;
    cv::Mat m_cameraMatrixNew;
    cv::Mat m_map1;
    cv::Mat m_map2;
    uint64_t m_framecount;
    apriltag_family_t* m_aprilTagFamily = nullptr;
    apriltag_detector_t* m_aprilTagDetector = nullptr;
    const AprilTagConfig m_aprilTagConfig;
    const AprilTagDetectorOptions m_aprilTagDetectorOptions;
};
} // perception
