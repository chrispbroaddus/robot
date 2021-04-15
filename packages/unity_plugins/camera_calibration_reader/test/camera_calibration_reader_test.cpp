//
// Created by byungsookim on 5/28/17.
//

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/unity_plugins/camera_calibration_reader/include/camera_calibration_reader.h"
#include "packages/unity_plugins/utils/include/zippy_image_interop.h"

using namespace unity_plugins;

TEST(CameraCalibrationReader, ctor) {
    const std::string testJsonFile("packages/unity_plugins/camera_calibration_reader/test/system_calibration_example.json");
    CameraCalibrationReader cameraCalibrationReader(testJsonFile);

    const auto calibrationFrontLeft = cameraCalibrationReader.get(hal::CameraId::FrontLeftStereo);

    EXPECT_EQ(calibrationFrontLeft.resolutionX, 2440);
    EXPECT_EQ(calibrationFrontLeft.resolutionY, 2046);
    EXPECT_NEAR(calibrationFrontLeft.focalLengthX, 1813.083, 1e-4);
    EXPECT_NEAR(calibrationFrontLeft.focalLengthY, 1809.95, 1e-4);
    EXPECT_NEAR(calibrationFrontLeft.centerX, 1231.583, 1e-4);
    EXPECT_NEAR(calibrationFrontLeft.centerY, 1076.001, 1e-4);
    EXPECT_NEAR(calibrationFrontLeft.kb4_0, -0.003544259, 1e-7);
    EXPECT_NEAR(calibrationFrontLeft.kb4_1, -0.0507795, 1e-7);
    EXPECT_NEAR(calibrationFrontLeft.kb4_2, 0.006967146, 1e-7);
    EXPECT_NEAR(calibrationFrontLeft.kb4_3, -0.0002875519, 1e-7);

    const auto calibrationFrontRight = cameraCalibrationReader.get(hal::CameraId::FrontRightStereo);

    EXPECT_EQ(calibrationFrontRight.resolutionX, 2440);
    EXPECT_EQ(calibrationFrontRight.resolutionY, 2046);
    EXPECT_NEAR(calibrationFrontRight.focalLengthX, 1829.234, 1e-4);
    EXPECT_NEAR(calibrationFrontRight.focalLengthY, 1827.453, 1e-4);
    EXPECT_NEAR(calibrationFrontRight.centerX, 1274.092, 1e-4);
    EXPECT_NEAR(calibrationFrontRight.centerY, 1021.1631, 1e-4);
    EXPECT_NEAR(calibrationFrontRight.kb4_0, -0.007749317, 1e-7);
    EXPECT_NEAR(calibrationFrontRight.kb4_1, -0.04870825, 1e-7);
    EXPECT_NEAR(calibrationFrontRight.kb4_2, 0.006656943, 1e-7);
    EXPECT_NEAR(calibrationFrontRight.kb4_3, -0.0002728968, 1e-7);
}