#include "packages/perception/fiducials/apriltag_detector.h"
#include "packages/core/include/chrono.h"
#include "packages/utilities/opencv/datatype_conversions.h"

#include "glog/logging.h"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "tag16h5.h"
#include "tag25h7.h"
#include "tag25h9.h"
#include "tag36h10.h"
#include "tag36h11.h"

DEFINE_string(apriltag_config, "", "Apriltag configuration.");
DEFINE_string(apriltag_detector_options, "", "AprilTag detector specific options.");

namespace perception {

constexpr uint64_t validPeriodInNanoSeconds = 33000000;

AprilTagConfig loadAprilTagConfig(const std::string& textPbFile) {
    AprilTagConfig config;
    std::ifstream t(textPbFile);
    std::stringstream buffer;
    buffer << t.rdbuf();
    CHECK(google::protobuf::TextFormat::MergeFromString(buffer.str(), &config));
    return config;
}

AprilTagConfig loadDefaultAprilTagConfig() { return loadAprilTagConfig(kDefaultAprilTagConfig); }

AprilTagDetectorOptions loadAprilTagDetectorOptions(const std::string& textPbFile) {
    perception::AprilTagDetectorOptions options;
    std::ifstream t(textPbFile);
    std::stringstream buffer;
    buffer << t.rdbuf();
    CHECK(google::protobuf::TextFormat::MergeFromString(buffer.str(), &options));
    return options;
}

AprilTagDetectorOptions loadDefaultAprilTagDetectorOptions() { return loadAprilTagDetectorOptions(kDefaultAprilTagDetectorOptions); }

void cameraSampleToMonochromeOcvMat(const hal::CameraSample& cameraSample, cv::Mat& image) {

    if (cameraSample.image().format() == hal::PB_LUMINANCE || cameraSample.image().format() == hal::PB_RAW) {
        OpenCVUtility::cameraSampleToOcvMat(cameraSample, image, false);
    } else if (cameraSample.image().format() == hal::PB_RGB) {
        cv::Mat temp;
        OpenCVUtility::cameraSampleToOcvMat(cameraSample, temp, false);
        cv::cvtColor(temp, image, CV_RGB2GRAY);
    } else if (cameraSample.image().format() == hal::PB_RGBA) {
        cv::Mat temp;
        OpenCVUtility::cameraSampleToOcvMat(cameraSample, temp, false);
        cv::cvtColor(temp, image, CV_RGBA2GRAY);
    } else {
        throw std::runtime_error("AprilTag Detector: Unsupported image format");
    }
}

std::string aprilTagFamilyString(AprilTagFamily aprilTagFamily) {
    std::stringstream stream;
    switch (aprilTagFamily) {
    case AprilTag36h11:
        stream << "AprilTag36h11";
        break;
    case AprilTag36h10:
        stream << "AprilTag36h10";
        break;
    case AprilTag25h9:
        stream << "AprilTag25h9";
        break;
    case AprilTag25h7:
        stream << "AprilTag25h7";
        break;
    case AprilTag16h5:
        stream << "AprilTag16h5";
        break;
    default:
        stream << "INVALID AprilTag Family";
    }
    return stream.str();
}

AprilTagDetector::AprilTagDetector(const calibration::CameraIntrinsicCalibration& cameraIntrinsicCalibration,
    const AprilTagConfig& aprilTagConfig, const AprilTagDetectorOptions& aprilTagDetectorOptions)
    : m_aprilTagConfig(aprilTagConfig)
    , m_aprilTagDetectorOptions(aprilTagDetectorOptions) {

    m_cameraMatrix = cv::Mat(3, 3, CV_64FC1, 0.0);
    m_distortion = cv::Mat(1, 4, CV_64FC1);

    if (cameraIntrinsicCalibration.scaledfocallengthx() <= 0 || cameraIntrinsicCalibration.scaledfocallengthy() <= 0
        || cameraIntrinsicCalibration.opticalcenterx() <= 0 || cameraIntrinsicCalibration.opticalcentery() <= 0) {
        LOG(ERROR) << "AprilTag Detector: Bad Intrinsic Calibration";
        throw std::runtime_error("AprilTag Detector: Bad Intrinsic Calibration");
    }

    m_cameraMatrix.at<double>(0, 0) = cameraIntrinsicCalibration.scaledfocallengthx();
    m_cameraMatrix.at<double>(0, 1) = cameraIntrinsicCalibration.skew();
    m_cameraMatrix.at<double>(0, 2) = cameraIntrinsicCalibration.opticalcenterx();
    m_cameraMatrix.at<double>(1, 1) = cameraIntrinsicCalibration.scaledfocallengthy();
    m_cameraMatrix.at<double>(1, 2) = cameraIntrinsicCalibration.opticalcentery();
    m_cameraMatrix.at<double>(2, 2) = 1;

    m_distortion.at<double>(0) = cameraIntrinsicCalibration.kannalabrandt().radialdistortioncoefficientk(0);
    m_distortion.at<double>(1) = cameraIntrinsicCalibration.kannalabrandt().radialdistortioncoefficientk(1);
    m_distortion.at<double>(2) = cameraIntrinsicCalibration.kannalabrandt().radialdistortioncoefficientk(2);
    m_distortion.at<double>(3) = cameraIntrinsicCalibration.kannalabrandt().radialdistortioncoefficientk(3);

    /// Since the Fisheye lenses have very large Fov (185 degree), we undistort a fraction of the entire image
    m_cameraMatrix.copyTo(m_cameraMatrixNew);
    m_cameraMatrixNew.at<double>(0, 0) *= 0.8;
    m_cameraMatrixNew.at<double>(1, 1) *= 0.8;

    cv::Size imageSize(cameraIntrinsicCalibration.resolutionx(), cameraIntrinsicCalibration.resolutiony());
    cv::fisheye::initUndistortRectifyMap(
        m_cameraMatrix, m_distortion, cv::Mat::eye(3, 3, CV_64FC1), m_cameraMatrixNew, imageSize, CV_16SC2, m_map1, m_map2);

    LOG(INFO) << m_aprilTagDetectorOptions.DebugString();

    // Initialize tag detector with options
    if (m_aprilTagConfig.apriltagfamily() == AprilTag36h11) {
        m_aprilTagFamily = tag36h11_create();
    } else if (m_aprilTagConfig.apriltagfamily() == AprilTag36h10) {
        m_aprilTagFamily = tag36h10_create();
    } else if (m_aprilTagConfig.apriltagfamily() == AprilTag25h9) {
        m_aprilTagFamily = tag25h9_create();
    } else if (m_aprilTagConfig.apriltagfamily() == AprilTag25h7) {
        m_aprilTagFamily = tag25h7_create();
    } else if (m_aprilTagConfig.apriltagfamily() == AprilTag16h5) {
        m_aprilTagFamily = tag16h5_create();
    } else {
        LOG(ERROR) << "Invalid AprilTag family";
        throw std::runtime_error("Invalid AprilTag family");
    }
    CHECK_NOTNULL(m_aprilTagFamily);
    m_aprilTagFamily->black_border = m_aprilTagConfig.border();

    m_aprilTagDetector = apriltag_detector_create();
    CHECK_NOTNULL(m_aprilTagDetector);
    apriltag_detector_add_family(m_aprilTagDetector, m_aprilTagFamily);
    m_aprilTagDetector->quad_decimate = m_aprilTagDetectorOptions.quaddecimate();
    m_aprilTagDetector->quad_sigma = m_aprilTagDetectorOptions.quadsigma();
    m_aprilTagDetector->nthreads = m_aprilTagDetectorOptions.nthreads();
    m_aprilTagDetector->debug = m_aprilTagDetectorOptions.debug();
    m_aprilTagDetector->refine_edges = m_aprilTagDetectorOptions.refineedges();
    m_aprilTagDetector->refine_decode = m_aprilTagDetectorOptions.refinedecode();
    m_aprilTagDetector->refine_pose = m_aprilTagDetectorOptions.refinepose();

    m_framecount = 0;

    LOG(INFO) << "AprilTag detector created";
}

bool AprilTagDetector::estimatePose(
    const hal::CameraSample& cameraSample, std::vector<calibration::CoordinateTransformation>& detectedPoses) {
    cv::Mat inputImage;
    cv::Mat undistortedImage;

    if (cameraSample.image().rows() == 0 || cameraSample.image().cols() == 0) {
        LOG(ERROR) << "AprilTag Detector: Invalid image size";
    }

    auto start = std::chrono::steady_clock::now();

    cameraSampleToMonochromeOcvMat(cameraSample, inputImage);

    cv::remap(inputImage, undistortedImage, m_map1, m_map2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);

    std::shared_ptr<image_u8_t> image(image_u8_create(undistortedImage.cols, undistortedImage.rows), &image_u8_destroy);

    for (int r = 0; r < undistortedImage.rows; ++r) {
        std::memcpy(image->buf + r * image->stride, undistortedImage.data + r * undistortedImage.step, undistortedImage.cols);
    }

    std::shared_ptr<zarray_t> detections(apriltag_detector_detect(m_aprilTagDetector, image.get()), &apriltag_detections_destroy);
    CHECK_NOTNULL(detections);

    for (int i = 0; i < zarray_size(detections.get()); i++) {

        apriltag_detection_t* det;
        zarray_get(detections.get(), i, &det);
        CHECK_NOTNULL(det);
        VLOG(2) << "April Tag ID: " << det->id;

        std::vector<cv::Point2d> imagePts;
        std::vector<cv::Point3d> objectPts;

        imagePts.push_back(cv::Point2d(det->p[0][0], det->p[0][1]));
        imagePts.push_back(cv::Point2d(det->p[1][0], det->p[1][1]));
        imagePts.push_back(cv::Point2d(det->p[2][0], det->p[2][1]));
        imagePts.push_back(cv::Point2d(det->p[3][0], det->p[3][1]));

        /// Defining the fiducial coordinate system
        /// Using a right-handed coordinated system with the origin at the center of the apriltag,
        /// +ve X-axis upwards, +ve Y-axis to the right and +ve Z-outwards
        float sideLengthInMeters = m_aprilTagConfig.sidelengthinmeters();
        float semiSideLengthInMeters = sideLengthInMeters / 2.0f;
        objectPts.push_back(cv::Point3d(-semiSideLengthInMeters, -semiSideLengthInMeters, 0));
        objectPts.push_back(cv::Point3d(-semiSideLengthInMeters, semiSideLengthInMeters, 0));
        objectPts.push_back(cv::Point3d(semiSideLengthInMeters, semiSideLengthInMeters, 0));
        objectPts.push_back(cv::Point3d(semiSideLengthInMeters, -semiSideLengthInMeters, 0));

        cv::Mat rvec;
        cv::Mat tvec;
        cv::solvePnP(objectPts, imagePts, m_cameraMatrixNew, cv::noArray(), rvec, tvec);
        VLOG(2) << "Detected AprilTag pose";
        VLOG(2) << "Rodrigues Vector: " << rvec;
        VLOG(2) << "Translation Vector: " << tvec;

        hal::Device* sourceDevice = new hal::Device();
        CHECK_NOTNULL(sourceDevice);
        hal::Device* targetDevice = new hal::Device();
        CHECK_NOTNULL(targetDevice);
        calibration::CoordinateFrame* sourceFrame = new calibration::CoordinateFrame();
        CHECK_NOTNULL(sourceFrame);
        calibration::CoordinateFrame* targetFrame = new calibration::CoordinateFrame();
        CHECK_NOTNULL(targetFrame);
        calibration::CoordinateTransformation transformation;

        const std::chrono::nanoseconds timestamp = core::chrono::gps::wallClockInNanoseconds();
        core::SystemTimestamp* validBeginSource = new core::SystemTimestamp();
        CHECK_NOTNULL(validBeginSource);
        validBeginSource->set_nanos(timestamp.count());
        core::SystemTimestamp* validEndSource = new core::SystemTimestamp();
        CHECK_NOTNULL(validEndSource);
        validEndSource->set_nanos(timestamp.count() + validPeriodInNanoSeconds);

        core::SystemTimestamp* validBeginTarget = new core::SystemTimestamp();
        CHECK_NOTNULL(validBeginTarget);
        validBeginTarget->set_nanos(timestamp.count());
        core::SystemTimestamp* validEndTarget = new core::SystemTimestamp();
        CHECK_NOTNULL(validEndTarget);
        validEndTarget->set_nanos(timestamp.count() + validPeriodInNanoSeconds);

        sourceDevice->set_name(aprilTagFamilyString(m_aprilTagConfig.apriltagfamily()));
        sourceDevice->set_serialnumber(det->id);
        sourceFrame->set_allocated_device(sourceDevice);
        sourceFrame->set_allocated_validperiodbegin(validBeginSource);
        sourceFrame->set_allocated_validperiodend(validEndSource);

        targetDevice->set_name(cameraSample.device().name());
        targetDevice->set_serialnumber(cameraSample.device().serialnumber());
        targetFrame->set_allocated_device(targetDevice);
        targetFrame->set_allocated_validperiodbegin(validBeginTarget);
        targetFrame->set_allocated_validperiodend(validEndTarget);

        transformation.set_allocated_sourcecoordinateframe(sourceFrame);
        transformation.set_allocated_targetcoordinateframe(targetFrame);
        transformation.set_rodriguesrotationx(rvec.at<double>(0));
        transformation.set_rodriguesrotationy(rvec.at<double>(1));
        transformation.set_rodriguesrotationz(rvec.at<double>(2));
        transformation.set_translationx(tvec.at<double>(0));
        transformation.set_translationy(tvec.at<double>(1));
        transformation.set_translationz(tvec.at<double>(2));
        transformation.set_timeoffsetnanoseconds(0);

        detectedPoses.push_back(transformation);
    }
    auto end = std::chrono::steady_clock::now();
    VLOG(2) << zarray_size(detections.get()) << " tags detected in "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms";

    m_framecount++;
    return true;
}

AprilTagDetector::~AprilTagDetector() {
    apriltag_detector_destroy(m_aprilTagDetector);
    if (m_aprilTagConfig.apriltagfamily() == AprilTag36h11) {
        tag36h11_destroy(m_aprilTagFamily);
    } else if (m_aprilTagConfig.apriltagfamily() == AprilTag36h10) {
        tag36h10_destroy(m_aprilTagFamily);
    } else if (m_aprilTagConfig.apriltagfamily() == AprilTag25h9) {
        tag25h9_destroy(m_aprilTagFamily);
    } else if (m_aprilTagConfig.apriltagfamily() == AprilTag25h7) {
        tag25h7_destroy(m_aprilTagFamily);
    } else if (m_aprilTagConfig.apriltagfamily() == AprilTag16h5) {
        tag16h5_destroy(m_aprilTagFamily);
    }
}
}
