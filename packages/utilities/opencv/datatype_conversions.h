#pragma once

#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"

#include "opencv2/core.hpp"

namespace OpenCVUtility {

/// Convert camera intrinsic calibration data from protobuf format to opencv Mat format
/// \param calibration  : Input CameraIntrinsicCalibration protobuf
/// \param cameraMatrix : Output camera matrix in opencv Mat format
/// \param distortion   : Output distortion coefficients in opencv Mat format
void cameraIntrinsicProtoToOcvMat(const calibration::CameraIntrinsicCalibration& calibration, cv::Mat& cameraMatrix, cv::Mat& distortion);

/// Convert coordinate transformations from protobuf format to opencv Mat format
/// \param transformation : Input CoordinateTranformation protobuf
/// \param rotationMatrix : Output rotation matrix in opencv Mat format
/// \param translation    : Output translation vector in opencv Mat format
void coordinateTransformationProtoToOcvMat(
    const calibration::CoordinateTransformation& transformation, cv::Mat& rotationMatrix, cv::Mat& translation);

/// Convert images from CameraSample format to opencv Mat format
/// \param cameraSample : Input CameraSample Image
/// \param image        : Outut image in opencv Mat format
/// \param copyData     : Input flag to specify whether the input image data should be copied(true) or shared(false)
void cameraSampleToOcvMat(const hal::CameraSample& cameraSample, cv::Mat& image, bool copyData);

/// Convert image from opencv Mat format to a Camera Sample image
/// \param image        : Input image in opencv Mat format
/// \param cameraSample : Output image in CameraSample format
void ocvMatToCameraSample(const cv::Mat& image, hal::CameraSample& cameraSample);
}
