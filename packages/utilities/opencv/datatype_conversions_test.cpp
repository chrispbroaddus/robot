#include "datatype_conversions.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "opencv2/calib3d.hpp"

using namespace cv;
using namespace OpenCVUtility;

TEST(DatatypeConversionsTest, canConvertPinholeCameraIntrinsics) {

    calibration::CameraIntrinsicCalibration intrinsicCalibration;
    calibration::PinholeCameraDistortionModel* pinholeCameraDistortionModel = new calibration::PinholeCameraDistortionModel();

    double scaledFocalLengthX = 100.1;
    double scaledFocalLengthY = 100.2;
    double opticalCenterX = 320.3;
    double opticalCenterY = 240.4;
    double skew = 0.5;

    intrinsicCalibration.set_scaledfocallengthx(scaledFocalLengthX);
    intrinsicCalibration.set_scaledfocallengthy(scaledFocalLengthY);
    intrinsicCalibration.set_opticalcenterx(opticalCenterX);
    intrinsicCalibration.set_opticalcentery(opticalCenterY);
    intrinsicCalibration.set_skew(skew);

    intrinsicCalibration.set_allocated_pinhole(pinholeCameraDistortionModel);

    Mat cameraMatrix;
    Mat distortion;
    cameraIntrinsicProtoToOcvMat(intrinsicCalibration, cameraMatrix, distortion);
    LOG(INFO) << "Conversion complete";

    EXPECT_EQ(3, cameraMatrix.rows);
    EXPECT_EQ(3, cameraMatrix.cols);
    EXPECT_EQ(scaledFocalLengthX, cameraMatrix.at<double>(0, 0));
    EXPECT_EQ(scaledFocalLengthY, cameraMatrix.at<double>(1, 1));
    EXPECT_EQ(opticalCenterX, cameraMatrix.at<double>(0, 2));
    EXPECT_EQ(opticalCenterY, cameraMatrix.at<double>(1, 2));
    EXPECT_EQ(skew, cameraMatrix.at<double>(0, 1));
    EXPECT_EQ(0, cameraMatrix.at<double>(1, 0));
    EXPECT_EQ(0, cameraMatrix.at<double>(2, 0));
    EXPECT_EQ(0, cameraMatrix.at<double>(2, 1));
    EXPECT_EQ(1, cameraMatrix.at<double>(2, 2));
    EXPECT_EQ(1, distortion.rows);
    EXPECT_EQ(4, distortion.cols);
    EXPECT_EQ(0, distortion.at<double>(0));
    EXPECT_EQ(0, distortion.at<double>(1));
    EXPECT_EQ(0, distortion.at<double>(2));
    EXPECT_EQ(0, distortion.at<double>(3));
}

TEST(DatatypeConversionsTest, canConvertKb4CameraIntrinsics) {

    calibration::CameraIntrinsicCalibration intrinsicCalibration;
    calibration::KannalaBrandtDistortionModel* kannalaBrandtDistortionModel = new calibration::KannalaBrandtDistortionModel();

    double scaledFocalLengthX = 100.1;
    double scaledFocalLengthY = 100.2;
    double opticalCenterX = 320.3;
    double opticalCenterY = 240.4;
    double skew = 0.5;

    intrinsicCalibration.set_scaledfocallengthx(scaledFocalLengthX);
    intrinsicCalibration.set_scaledfocallengthy(scaledFocalLengthY);
    intrinsicCalibration.set_opticalcenterx(opticalCenterX);
    intrinsicCalibration.set_opticalcentery(opticalCenterY);
    intrinsicCalibration.set_skew(skew);

    double k1 = 0.1;
    double k2 = 0.2;
    double k3 = 0.3;
    double k4 = 0.4;
    kannalaBrandtDistortionModel->add_radialdistortioncoefficientk(k1);
    kannalaBrandtDistortionModel->add_radialdistortioncoefficientk(k2);
    kannalaBrandtDistortionModel->add_radialdistortioncoefficientk(k3);
    kannalaBrandtDistortionModel->add_radialdistortioncoefficientk(k4);
    intrinsicCalibration.set_allocated_kannalabrandt(kannalaBrandtDistortionModel);

    Mat cameraMatrix;
    Mat distortion;
    cameraIntrinsicProtoToOcvMat(intrinsicCalibration, cameraMatrix, distortion);

    EXPECT_EQ(3, cameraMatrix.rows);
    EXPECT_EQ(3, cameraMatrix.cols);
    EXPECT_EQ(scaledFocalLengthX, cameraMatrix.at<double>(0, 0));
    EXPECT_EQ(scaledFocalLengthY, cameraMatrix.at<double>(1, 1));
    EXPECT_EQ(opticalCenterX, cameraMatrix.at<double>(0, 2));
    EXPECT_EQ(opticalCenterY, cameraMatrix.at<double>(1, 2));
    EXPECT_EQ(skew, cameraMatrix.at<double>(0, 1));
    EXPECT_EQ(0, cameraMatrix.at<double>(1, 0));
    EXPECT_EQ(0, cameraMatrix.at<double>(2, 0));
    EXPECT_EQ(0, cameraMatrix.at<double>(2, 1));
    EXPECT_EQ(1, cameraMatrix.at<double>(2, 2));
    EXPECT_EQ(1, distortion.rows);
    EXPECT_EQ(4, distortion.cols);
    EXPECT_EQ(k1, distortion.at<double>(0));
    EXPECT_EQ(k2, distortion.at<double>(1));
    EXPECT_EQ(k3, distortion.at<double>(2));
    EXPECT_EQ(k4, distortion.at<double>(3));
}

TEST(DatatypeConversionsTest, canConvertCoordinateTransformation) {

    calibration::CoordinateTransformation coordinateTransformation;

    double angle = M_PI / 3;
    Mat rotationMatrix = Mat(3, 3, CV_64FC1);
    rotationMatrix.at<double>(0, 0) = std::cos(angle);
    rotationMatrix.at<double>(0, 1) = std::sin(angle);
    rotationMatrix.at<double>(0, 2) = 0;
    rotationMatrix.at<double>(1, 0) = -std::sin(angle);
    rotationMatrix.at<double>(1, 1) = std::cos(angle);
    rotationMatrix.at<double>(1, 2) = 0;
    rotationMatrix.at<double>(2, 0) = 0;
    rotationMatrix.at<double>(2, 1) = 0;
    rotationMatrix.at<double>(2, 2) = 1;

    Mat rvec;
    Rodrigues(rotationMatrix, rvec);

    LOG(INFO) << rvec;

    coordinateTransformation.set_rodriguesrotationx(rvec.at<double>(0));
    coordinateTransformation.set_rodriguesrotationy(rvec.at<double>(1));
    coordinateTransformation.set_rodriguesrotationz(rvec.at<double>(2));

    double translationx = 1.1;
    double translationy = 1.2;
    double translationz = 1.3;
    coordinateTransformation.set_translationx(translationx);
    coordinateTransformation.set_translationy(translationy);
    coordinateTransformation.set_translationz(translationz);

    Mat convertedRotationMatrix;
    Mat translationMarix;
    coordinateTransformationProtoToOcvMat(coordinateTransformation, convertedRotationMatrix, translationMarix);

    EXPECT_NEAR(rotationMatrix.at<double>(0, 0), convertedRotationMatrix.at<double>(0, 0), 0.000001);
    EXPECT_NEAR(rotationMatrix.at<double>(0, 1), convertedRotationMatrix.at<double>(0, 1), 0.000001);
    EXPECT_NEAR(rotationMatrix.at<double>(0, 2), convertedRotationMatrix.at<double>(0, 2), 0.000001);
    EXPECT_NEAR(rotationMatrix.at<double>(1, 0), convertedRotationMatrix.at<double>(1, 0), 0.000001);
    EXPECT_NEAR(rotationMatrix.at<double>(1, 1), convertedRotationMatrix.at<double>(1, 1), 0.000001);
    EXPECT_NEAR(rotationMatrix.at<double>(1, 2), convertedRotationMatrix.at<double>(1, 2), 0.000001);
    EXPECT_NEAR(rotationMatrix.at<double>(2, 0), convertedRotationMatrix.at<double>(2, 0), 0.000001);
    EXPECT_NEAR(rotationMatrix.at<double>(2, 1), convertedRotationMatrix.at<double>(2, 1), 0.000001);
    EXPECT_NEAR(rotationMatrix.at<double>(2, 2), convertedRotationMatrix.at<double>(2, 2), 0.000001);
    EXPECT_EQ(translationx, translationMarix.at<double>(0));
    EXPECT_EQ(translationy, translationMarix.at<double>(1));
    EXPECT_EQ(translationz, translationMarix.at<double>(2));
}

TEST(DatatypeConversionsTest, canConvertMonochromeCameraSampleToOcvWithCopy) {

    hal::CameraSample cameraSample;
    hal::Image* image = new hal::Image();

    image->set_type(hal::PB_UNSIGNED_BYTE);
    image->set_format(hal::PB_LUMINANCE);
    image->set_rows(10);
    image->set_cols(10);
    image->set_stride(10);

    std::vector<unsigned char> imageData(300, 128);
    image->set_data(imageData.data(), imageData.size());

    cameraSample.set_allocated_image(image);

    Mat ocvImage;
    cameraSampleToOcvMat(cameraSample, ocvImage, true);

    EXPECT_EQ(cameraSample.image().rows(), ocvImage.rows);
    EXPECT_EQ(cameraSample.image().cols(), ocvImage.cols);
    EXPECT_EQ(CV_8UC1, ocvImage.type());
    EXPECT_NE((unsigned char*)(cameraSample.image().data().data()), ocvImage.data);
    EXPECT_EQ(128, *ocvImage.data);
    EXPECT_EQ(128, *(ocvImage.data + 99));
}

TEST(DatatypeConversionsTest, canConvertMonochromeCameraSampleToOcvWithoutCopy) {

    hal::CameraSample cameraSample;
    hal::Image* image = new hal::Image();

    image->set_type(hal::PB_UNSIGNED_BYTE);
    image->set_format(hal::PB_LUMINANCE);
    image->set_rows(10);
    image->set_cols(10);
    image->set_stride(10);

    std::vector<unsigned char> imageData(100, 128);
    image->set_data(imageData.data(), imageData.size());

    cameraSample.set_allocated_image(image);

    Mat ocvImage;
    cameraSampleToOcvMat(cameraSample, ocvImage, false);

    EXPECT_EQ(cameraSample.image().rows(), ocvImage.rows);
    EXPECT_EQ(cameraSample.image().cols(), ocvImage.cols);
    EXPECT_EQ(CV_8UC1, ocvImage.type());
    EXPECT_EQ((unsigned char*)(cameraSample.image().data().data()), ocvImage.data);
    EXPECT_EQ(128, *ocvImage.data);
    EXPECT_EQ(128, *(ocvImage.data + 99));
}

TEST(DatatypeConversionsTest, canConvertRGBCameraSampleToOcvWithCopy) {

    hal::CameraSample cameraSample;
    hal::Image* image = new hal::Image();

    image->set_type(hal::PB_UNSIGNED_BYTE);
    image->set_format(hal::PB_RGB);
    image->set_rows(10);
    image->set_cols(10);
    image->set_stride(10 * 3);

    std::vector<unsigned char> imageData(300, 128);
    image->set_data(imageData.data(), imageData.size());

    cameraSample.set_allocated_image(image);

    Mat ocvImage;
    cameraSampleToOcvMat(cameraSample, ocvImage, true);

    EXPECT_EQ(cameraSample.image().rows(), ocvImage.rows);
    EXPECT_EQ(cameraSample.image().cols(), ocvImage.cols);
    EXPECT_EQ(CV_8UC3, ocvImage.type());
    EXPECT_NE((unsigned char*)(cameraSample.image().data().data()), ocvImage.data);
    EXPECT_EQ(128, *ocvImage.data);
    EXPECT_EQ(128, *(ocvImage.data + 299));
}

TEST(DatatypeConversionsTest, canConvertMonochromeOcvImageToCameraSample) {

    Mat ocvImage = Mat(100, 100, CV_8UC1, 128);

    hal::CameraSample cameraSample;
    ocvMatToCameraSample(ocvImage, cameraSample);

    EXPECT_EQ(ocvImage.rows, cameraSample.image().rows());
    EXPECT_EQ(ocvImage.cols, cameraSample.image().cols());
    EXPECT_EQ(hal::PB_UNSIGNED_BYTE, cameraSample.image().type());
    EXPECT_EQ(hal::PB_LUMINANCE, cameraSample.image().format());
    EXPECT_EQ(128, (unsigned char)*(cameraSample.image().data().data()));
    EXPECT_EQ(128, (unsigned char)*(cameraSample.image().data().data() + 99));
}

TEST(DatatypeConversionsTest, canConvertRGBOcvImageToCameraSample) {

    Mat ocvImage = Mat(100, 100, CV_8UC3, cv::Vec3b(128, 133, 201));

    hal::CameraSample cameraSample;
    ocvMatToCameraSample(ocvImage, cameraSample);

    EXPECT_EQ(ocvImage.rows, cameraSample.image().rows());
    EXPECT_EQ(ocvImage.cols, cameraSample.image().cols());
    EXPECT_EQ(hal::PB_UNSIGNED_BYTE, cameraSample.image().type());
    EXPECT_EQ(hal::PB_RGB, cameraSample.image().format());
    EXPECT_EQ(128, (unsigned char)*(cameraSample.image().data().data()));
    EXPECT_EQ(133, (unsigned char)*(cameraSample.image().data().data() + 1));
    EXPECT_EQ(201, (unsigned char)*(cameraSample.image().data().data() + 2));
    EXPECT_EQ(201, (unsigned char)*(cameraSample.image().data().data() + 299));
}

TEST(DatatypeConversionsTest, canConvert1ChannelFloatOcvImageToCameraSample) {

    Mat ocvImage = Mat(100, 100, CV_32FC1, 1.23);

    hal::CameraSample cameraSample;
    ocvMatToCameraSample(ocvImage, cameraSample);

    EXPECT_EQ(ocvImage.rows, cameraSample.image().rows());
    EXPECT_EQ(ocvImage.cols, cameraSample.image().cols());
    EXPECT_EQ(hal::PB_FLOAT, cameraSample.image().type());
    EXPECT_EQ(hal::PB_RANGE, cameraSample.image().format());
    EXPECT_NEAR(1.23, *((float*)cameraSample.image().data().data()), 0.000001);
    EXPECT_NEAR(1.23, *((float*)cameraSample.image().data().data() + 99), 0.000001);
}

TEST(DatatypeConversionsTest, canConvert3ChannelFloatOcvImageToCameraSample) {

    Mat ocvImage = Mat(100, 100, CV_32FC3, cv::Vec3f(0.1, 1.23, 0.48));

    hal::CameraSample cameraSample;
    ocvMatToCameraSample(ocvImage, cameraSample);

    EXPECT_EQ(ocvImage.rows, cameraSample.image().rows());
    EXPECT_EQ(ocvImage.cols, cameraSample.image().cols());
    EXPECT_EQ(hal::PB_FLOAT, cameraSample.image().type());
    EXPECT_EQ(hal::PB_POINTCLOUD, cameraSample.image().format());
    EXPECT_NEAR(0.1, *((float*)cameraSample.image().data().data()), 0.000001);
    EXPECT_NEAR(1.23, *((float*)cameraSample.image().data().data() + 1), 0.000001);
    EXPECT_NEAR(0.48, *((float*)cameraSample.image().data().data() + 2), 0.000001);
    EXPECT_NEAR(0.48, *((float*)cameraSample.image().data().data() + 299), 0.000001);
}
