#include "packages/perception/fiducials/apriltag_detector.h"

#include "glog/logging.h"
#include "opencv2/highgui.hpp"
#include "gtest/gtest.h"

using namespace perception;

namespace {

void OcvMatToDepthCameraSample(const cv::Mat& image, hal::CameraSample& cameraSample) {

    if (image.type() != CV_8UC1 && image.type() != CV_8UC3) {
        LOG(ERROR) << "Apriltag Test: Unsupported image format";
        throw std::runtime_error("Apriltag Test: Unsupported image format");
    }

    int bytes_per_pixel;

    // Device
    hal::Device* device = new hal::Device();
    CHECK_NOTNULL(device);
    device->set_name("depth");
    device->set_serialnumber(0);

    // Image
    hal::Image* halImage = new hal::Image();
    CHECK_NOTNULL(halImage);
    halImage->set_cols(static_cast<uint32_t>(image.cols));
    halImage->set_rows(static_cast<uint32_t>(image.rows));
    if (image.type() == CV_8UC1) {
        halImage->set_type(hal::PB_UNSIGNED_BYTE); // PB_DATA_TYPE
        halImage->set_format(hal::PB_LUMINANCE); // PB_RANGE
        halImage->set_stride(static_cast<uint32_t>(image.cols));
        bytes_per_pixel = 1;
    } else if (image.type() == CV_8UC3) {
        halImage->set_type(hal::PB_UNSIGNED_BYTE); // PB_DATA_TYPE
        halImage->set_format(hal::PB_RGB); // PB_RANGE
        bytes_per_pixel = 3;
        halImage->set_stride(static_cast<uint32_t>(image.cols * bytes_per_pixel));
    } else {
        LOG(ERROR) << "Apriltag Test: Unsupported image format";
        throw std::runtime_error("Apriltag Test: Unsupported image format");
    }

    halImage->set_data(image.data, static_cast<uint32_t>(image.rows * image.cols * bytes_per_pixel));

    // Timestamps
    core::HardwareTimestamp* hardwareTimestamp = new core::HardwareTimestamp();
    CHECK_NOTNULL(hardwareTimestamp);
    core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
    CHECK_NOTNULL(systemTimestamp);

    // Set the CameraSample
    cameraSample.set_allocated_device(device);
    cameraSample.set_allocated_image(halImage);
    cameraSample.set_allocated_systemtimestamp(systemTimestamp);
    cameraSample.set_allocated_hardwaretimestamp(hardwareTimestamp);
    cameraSample.set_id(0);
}

class AprilTagDetectorTest : public ::testing::Test {
protected:
    void SetUp() {

        hal::Device* camera = new hal::Device();
        camera->set_name("camera");
        camera->set_serialnumber(0);
        calibration::KannalaBrandtDistortionModel* kannalaBrandtDistortionModel = new calibration::KannalaBrandtDistortionModel();
        kannalaBrandtDistortionModel->add_radialdistortioncoefficientk(0.1374372);
        kannalaBrandtDistortionModel->add_radialdistortioncoefficientk(-0.100687);
        kannalaBrandtDistortionModel->add_radialdistortioncoefficientk(0.01317122);
        kannalaBrandtDistortionModel->add_radialdistortioncoefficientk(-0.0005528795);
        m_cameraIntrinsicCalibration.set_allocated_cameraundercalibration(camera);
        m_cameraIntrinsicCalibration.set_scaledfocallengthx(765.4143);
        m_cameraIntrinsicCalibration.set_scaledfocallengthy(761.2293);
        m_cameraIntrinsicCalibration.set_opticalcenterx(1259.358);
        m_cameraIntrinsicCalibration.set_opticalcentery(1072.122);
        m_cameraIntrinsicCalibration.set_resolutionx(2440);
        m_cameraIntrinsicCalibration.set_resolutiony(2446);
        m_cameraIntrinsicCalibration.set_skew(0.0);
        m_cameraIntrinsicCalibration.set_allocated_kannalabrandt(kannalaBrandtDistortionModel);

        cv::Mat ocvImage = cv::imread("packages/perception/fiducials/test/apriltag_test_image.png", 0);
        OcvMatToDepthCameraSample(ocvImage, m_testImage);

        LOG(INFO) << "Setup Complete";
    }

    void TearDown() { LOG(INFO) << "Teardown Complete"; }

    hal::CameraSample m_testImage;
    calibration::CameraIntrinsicCalibration m_cameraIntrinsicCalibration;
};
}

TEST_F(AprilTagDetectorTest, happyPath) {

    AprilTagDetectorOptions aprilTagDetectorOptions;
    AprilTagConfig aprilTagConfig;

    aprilTagConfig.set_apriltagfamily(perception::AprilTagFamily::AprilTag36h11);
    aprilTagConfig.set_border(1);
    aprilTagConfig.set_sidelengthinmeters(0.24);
    aprilTagDetectorOptions.set_debug(false);
    aprilTagDetectorOptions.set_nthreads(1);
    aprilTagDetectorOptions.set_quaddecimate(4.0);
    aprilTagDetectorOptions.set_quadsigma(0.0);
    aprilTagDetectorOptions.set_refinedecode(0);
    aprilTagDetectorOptions.set_refineedges(0);
    aprilTagDetectorOptions.set_refinepose(0);

    AprilTagDetector aprilTagDetector(m_cameraIntrinsicCalibration, aprilTagConfig, aprilTagDetectorOptions);

    std::vector<calibration::CoordinateTransformation> poses;
    aprilTagDetector.estimatePose(m_testImage, poses);

    EXPECT_EQ(poses.size(), 1);
    EXPECT_NEAR(poses[0].translationx(), 0.0, 0.05);
    EXPECT_NEAR(poses[0].translationy(), 0.0, 0.05);
    EXPECT_NEAR(poses[0].translationz(), 0.75, 0.1);
    double angle
        = sqrt(pow(poses[0].rodriguesrotationx(), 2) + pow(poses[0].rodriguesrotationy(), 2) + pow(poses[0].rodriguesrotationz(), 2))
        * 180.0 / 3.14;
    EXPECT_NEAR(angle, 0, 2);
}
