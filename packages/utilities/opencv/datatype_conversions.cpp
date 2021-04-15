#include "datatype_conversions.h"

#include "glog/logging.h"
#include "opencv2/calib3d.hpp"

namespace OpenCVUtility {

/// Convert camera intrinsic calibration data from protobuf format to opencv Mat format
/// \param calibration  : Input CameraIntrinsicCalibration protobuf
/// \param cameraMatrix : Output camera matrix in opencv Mat format
/// \param distortion   : Output distortion coefficients in opencv Mat format
void cameraIntrinsicProtoToOcvMat(const calibration::CameraIntrinsicCalibration& calibration, cv::Mat& cameraMatrix, cv::Mat& distortion) {

    cameraMatrix = cv::Mat(3, 3, CV_64FC1, 0.0);

    cameraMatrix.at<double>(0, 0) = calibration.scaledfocallengthx();
    cameraMatrix.at<double>(0, 1) = calibration.skew();
    cameraMatrix.at<double>(0, 2) = calibration.opticalcenterx();
    cameraMatrix.at<double>(1, 1) = calibration.scaledfocallengthy();
    cameraMatrix.at<double>(1, 2) = calibration.opticalcentery();
    cameraMatrix.at<double>(2, 2) = 1;

    if (calibration.has_kannalabrandt()) {
        distortion = cv::Mat(1, 4, CV_64FC1);
        distortion.at<double>(0) = calibration.kannalabrandt().radialdistortioncoefficientk(0);
        distortion.at<double>(1) = calibration.kannalabrandt().radialdistortioncoefficientk(1);
        distortion.at<double>(2) = calibration.kannalabrandt().radialdistortioncoefficientk(2);
        distortion.at<double>(3) = calibration.kannalabrandt().radialdistortioncoefficientk(3);
    } else if (calibration.has_pinhole()) {
        distortion = cv::Mat(1, 4, CV_64FC1, 0.0);
    } else {
        LOG(ERROR) << "Failed to convert camera intrinsics to opencv format: Unsupported distortion model";
        throw std::runtime_error("Failed to convert camera intrinsics to opencv format: Unsupported distortion model");
    }
}

/// Convert coordinate transformations from protobuf format to opencv Mat format
/// \param transformation : Input CoordinateTranformation protobuf
/// \param rotationMatrix : Output rotation matrix in opencv Mat format
/// \param translation    : Output translation vector in opencv Mat format
void coordinateTransformationProtoToOcvMat(
    const calibration::CoordinateTransformation& transformation, cv::Mat& rotationMatrix, cv::Mat& translation) {

    translation = cv::Mat(3, 1, CV_64FC1);
    cv::Mat rvec = cv::Mat(3, 1, CV_64FC1);

    rvec.at<double>(0) = transformation.rodriguesrotationx();
    rvec.at<double>(1) = transformation.rodriguesrotationy();
    rvec.at<double>(2) = transformation.rodriguesrotationz();

    cv::Rodrigues(rvec, rotationMatrix);

    translation.at<double>(0) = transformation.translationx();
    translation.at<double>(1) = transformation.translationy();
    translation.at<double>(2) = transformation.translationz();
}

/// Convert images from CameraSample format to opencv Mat format
/// \param cameraSample : Input CameraSample Image
/// \param image        : Outut image in opencv Mat format
/// \param copyData     : Input flag to specify whether the input image data should be copied(true) or shared(false)
void cameraSampleToOcvMat(const hal::CameraSample& cameraSample, cv::Mat& image, bool copyData) {

    int rows = cameraSample.image().rows();
    int cols = cameraSample.image().cols();
    int stride = cameraSample.image().stride();
    unsigned char* imageData = (unsigned char*)(cameraSample.image().data().data());
    CHECK_NOTNULL(imageData);

    if (cameraSample.image().type() == hal::PB_UNSIGNED_BYTE || cameraSample.image().type() == hal::PB_BYTE) {
        if (cameraSample.image().format() == hal::PB_LUMINANCE || cameraSample.image().format() == hal::PB_RAW) {
            if (copyData) {
                image = cv::Mat(rows, cols, CV_8UC1);
                std::memcpy(image.data, imageData, (size_t)rows * stride);
            } else {
                image = cv::Mat(rows, cols, CV_8UC1, imageData, (size_t)stride);
            }
        } else if (cameraSample.image().format() == hal::PB_RGB) {
            if (copyData) {
                image = cv::Mat(rows, cols, CV_8UC3);
                std::memcpy(image.data, imageData, (size_t)rows * stride);
            } else {
                image = cv::Mat(rows, cols, CV_8UC3, imageData, (size_t)stride);
            }
        } else if (cameraSample.image().format() == hal::PB_RGBA) {
            if (copyData) {
                image = cv::Mat(rows, cols, CV_8UC4);
                std::memcpy(image.data, imageData, (size_t)rows * stride);
            } else {
                image = cv::Mat(rows, cols, CV_8UC4, imageData, (size_t)stride);
            }
        } else {
            LOG(ERROR) << "Camera Sample to opencv Mat: Unsupported image format";
            throw std::runtime_error("Camera Sample to opencv Mat: Unsupported image format");
        }
    } else {
        LOG(ERROR) << "Camera Sample to opencv Mat: Unsupported image type";
        throw std::runtime_error("Camera Sample to opencv Mat: Unsupported image type");
    }
}

/// Convert image from opencv Mat format to a Camera Sample image
/// \param image        : Input image in opencv Mat format
/// \param cameraSample : Output image in CameraSample format
void ocvMatToCameraSample(const cv::Mat& image, hal::CameraSample& cameraSample) {

    if (image.type() != CV_8UC1 && image.type() != CV_8UC3 && image.type() != CV_8UC4 && image.type() != CV_32FC1
        && image.type() != CV_32FC3) {
        LOG(ERROR) << "Opencv Mat to CameraSample: Unsupported image format";
        throw std::runtime_error("Opencv Mat to CameraSample: Unsupported image format");
    }

    int bytes_per_pixel;
    // Image
    hal::Image* halImage = new hal::Image();
    CHECK_NOTNULL(halImage);
    halImage->set_cols((uint32_t)image.cols);
    halImage->set_rows((uint32_t)image.rows);
    if (image.type() == CV_8UC1) {
        halImage->set_type(hal::PB_UNSIGNED_BYTE);
        halImage->set_format(hal::PB_LUMINANCE);
        bytes_per_pixel = 1;
        halImage->set_stride((uint32_t)image.cols * bytes_per_pixel);
    } else if (image.type() == CV_8UC3) {
        halImage->set_type(hal::PB_UNSIGNED_BYTE);
        halImage->set_format(hal::PB_RGB);
        bytes_per_pixel = 3;
        halImage->set_stride((uint32_t)image.cols * bytes_per_pixel);
    } else if (image.type() == CV_8UC4) {
        halImage->set_type(hal::PB_UNSIGNED_BYTE);
        halImage->set_format(hal::PB_RGBA);
        bytes_per_pixel = 4;
        halImage->set_stride((uint32_t)image.cols * bytes_per_pixel);
    } else if (image.type() == CV_32FC1) {
        halImage->set_type(hal::PB_FLOAT);
        halImage->set_format(hal::PB_RANGE);
        bytes_per_pixel = 4;
        halImage->set_stride((uint32_t)image.cols * bytes_per_pixel);
    } else if (image.type() == CV_32FC3) {
        halImage->set_type(hal::PB_FLOAT);
        halImage->set_format(hal::PB_POINTCLOUD);
        bytes_per_pixel = 12;
        halImage->set_stride((uint32_t)image.cols * bytes_per_pixel);
    } else {
        LOG(ERROR) << "Opencv Mat to CameraSample: Unsupported image format";
        throw std::runtime_error("Opencv Mat to CameraSample: Unsupported image format");
    }

    halImage->set_data(image.data, (size_t)image.rows * image.cols * bytes_per_pixel);
    cameraSample.set_allocated_image(halImage);
}
}
