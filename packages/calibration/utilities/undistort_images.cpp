#include "packages/calibration/include/kb4_image_undistortion.h"
#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/hal/include/drivers/cameras/flycapture/flycapture_driver_factory.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/utilities/opencv/datatype_conversions.h"

#include "gflags/gflags.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/util/json_util.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <fstream>
#include <iostream>

DEFINE_string(cameraSerialNo, "", "camera serial number");
DEFINE_string(systemCalibrationFile, "", "system calibration file");
DEFINE_int32(numImages, 10000, "number of images to capture");
DEFINE_int32(left, 0, "leftmost corner of image sensor roi");
DEFINE_int32(top, 0, "topmost corner of image sensor roi");
DEFINE_int32(width, 2448, "width of image sensor roi");
DEFINE_int32(height, 2048, "height of image sensor roi");
DEFINE_double(framerate, 30, "framerate");
DEFINE_string(pixelFormat, "MONO8", "pixelformat ex: MONO8 / RGB8");
DEFINE_string(externalTrigger, "OFF", "external trigger ON / OFF");
DEFINE_string(format7Mode, "MODE_0", "Format7Mode ex: MODE0");

using namespace hal;

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    gflags::SetUsageMessage("Undistort Images");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    if (FLAGS_cameraSerialNo.empty() || FLAGS_systemCalibrationFile.empty()) {
        gflags::ShowUsageWithFlagsRestrict(argv[0], "packages/calibration/utilities/undistort_images.cpp");
        return 0;
    }

    calibration::SystemCalibration systemCalibration;
    std::ifstream file(FLAGS_systemCalibrationFile);
    std::stringstream buffer;
    buffer << file.rdbuf();
    if (buffer.str().size() == 0) {
        LOG(ERROR) << "Empty system calibration file";
        throw std::runtime_error("Empty system calibration file");
    }
    google::protobuf::util::JsonStringToMessage(buffer.str(), &systemCalibration);

    Eigen::Matrix<float, 3, 3> K;
    Eigen::Matrix<float, 4, 1> coef;
    bool calibrationFound = false;
    for (int i = 0; i < systemCalibration.cameraintrinsiccalibration_size(); i++) {
        if (systemCalibration.cameraintrinsiccalibration(i).cameraundercalibration().serialnumber() == std::stoul(FLAGS_cameraSerialNo)) {
            calibrationFound = true;
            K << systemCalibration.cameraintrinsiccalibration(i).scaledfocallengthx(),
                systemCalibration.cameraintrinsiccalibration(i).skew(), systemCalibration.cameraintrinsiccalibration(i).opticalcenterx(), 0,
                systemCalibration.cameraintrinsiccalibration(i).scaledfocallengthy(),
                systemCalibration.cameraintrinsiccalibration(i).opticalcentery(), 0, 0, 1;
            coef << systemCalibration.cameraintrinsiccalibration(i).kannalabrandt().radialdistortioncoefficientk(0),
                systemCalibration.cameraintrinsiccalibration(i).kannalabrandt().radialdistortioncoefficientk(1),
                systemCalibration.cameraintrinsiccalibration(i).kannalabrandt().radialdistortioncoefficientk(2),
                systemCalibration.cameraintrinsiccalibration(i).kannalabrandt().radialdistortioncoefficientk(3);
        }
    }

    std::cout << "K:\n" << K << std::endl;
    std::cout << "coef:\n" << coef.transpose() << std::endl;

    if (!calibrationFound) {
        throw std::runtime_error("No calibration found for camera: " + FLAGS_cameraSerialNo);
    }

    hal::details::property_map_t cameraConfig;
    cameraConfig["serialNumber"] = FLAGS_cameraSerialNo;
    cameraConfig["grabMode"] = "DROP_FRAMES";
    cameraConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    cameraConfig["format7Mode"] = FLAGS_format7Mode;
    cameraConfig["pixelFormat"] = FLAGS_pixelFormat;
    cameraConfig["width"] = std::to_string(FLAGS_width);
    cameraConfig["height"] = std::to_string(FLAGS_height);
    cameraConfig["left"] = std::to_string(FLAGS_left);
    cameraConfig["top"] = std::to_string(FLAGS_top);
    cameraConfig["numBuffers"] = std::to_string(10);
    cameraConfig["fps"] = std::to_string(FLAGS_framerate);
    cameraConfig["externalTrigger"] = FLAGS_externalTrigger;
    cameraConfig["AERoILeft"] = "0";
    cameraConfig["AERoITop"] = "0";
    cameraConfig["AERoIWidth"] = "2448";
    cameraConfig["AERoIHeight"] = "2048";

    FlycaptureDriverFactory flycaptureDriverFactory;
    std::shared_ptr<CameraDeviceInterface> camera = flycaptureDriverFactory.create(cameraConfig);

    CameraSample image;
    Eigen::Matrix<float, 3, 3> newK;
    float newFocalLength = 0.5f * K(0, 0);
    uint32_t outputRows = 1024;
    uint32_t outputCols = 1224;
    newK << newFocalLength, 0, outputCols / 2, 0, newFocalLength, outputRows / 2, 0, 0, 1;
    calibration::Kb4ImageUndistortion<float> kb4ImageUndistortion(K, coef, outputRows, outputCols, newK);
    for (int i = 0; i < FLAGS_numImages; i++) {
        if (camera->capture(image)) {
            hal::CameraSample undistortedImage;
            kb4ImageUndistortion.undistortImage(image.image(), *undistortedImage.mutable_image());
            cv::Mat ocvImage;
            cv::Mat ocvUndistortedImage;
            OpenCVUtility::cameraSampleToOcvMat(image, ocvImage, false);
            OpenCVUtility::cameraSampleToOcvMat(undistortedImage, ocvUndistortedImage, true);
            if (ocvUndistortedImage.channels() != 1) {
                cv::cvtColor(ocvUndistortedImage, ocvUndistortedImage, CV_BGR2RGB);
            }
            if (ocvImage.channels() != 1) {
                cv::cvtColor(ocvImage, ocvImage, CV_BGR2RGB);
            }
            cv::namedWindow("Distorted", 0);
            cv::imshow("Distorted", ocvImage);
            cv::namedWindow("Undistorted", 0);
            cv::imshow("Undistorted", ocvUndistortedImage);
            cv::waitKey(10);
        }
    }

    return 0;
}
