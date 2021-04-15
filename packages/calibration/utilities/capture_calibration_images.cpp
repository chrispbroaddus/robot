#include "packages/hal/include/drivers/cameras/flycapture/flycapture_driver_factory.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/utilities/opencv/datatype_conversions.h"

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <sys/stat.h>
#include <sys/types.h>

DEFINE_string(cameraSerialNo, "", "camera serial number");
DEFINE_string(outputFolder, "", "output folder path");
DEFINE_int32(numImages, 100, "number of images to capture");
DEFINE_int32(left, 0, "leftmost corner of image sensor roi");
DEFINE_int32(top, 0, "topmost corner of image sensor roi");
DEFINE_int32(width, 2448, "width of image sensor roi");
DEFINE_int32(height, 2048, "height of image sensor roi");
DEFINE_double(framerate, 2, "framerate");
DEFINE_string(pixelFormat, "MONO8", "pixelformat ex: MONO8 / RGB8");
DEFINE_string(externalTrigger, "OFF", "external trigger ON / OFF");
DEFINE_string(format7Mode, "MODE_0", "Format7Mode ex: MODE0");

using namespace hal;

int main(int argc, char* argv[]) {
    google::InitGoogleLogging(argv[0]);

    gflags::SetUsageMessage("Capture Calibration Images");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    if (FLAGS_cameraSerialNo.empty() || FLAGS_outputFolder.empty()) {
        gflags::ShowUsageWithFlagsRestrict(argv[0], "packages/calibration/utilities/capture_calibration_images.cpp");
        return 0;
    }

    struct stat info;
    if (stat(FLAGS_outputFolder.c_str(), &info) != 0 || !(info.st_mode & S_IFDIR)) {
        throw std::runtime_error("Invalid output folder");
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

    FlycaptureDriverFactory flycaptureDriverFactory;
    std::shared_ptr<CameraDeviceInterface> camera = flycaptureDriverFactory.create(cameraConfig);

    CameraSample image;
    for (int i = 0; i < FLAGS_numImages;) {
        if (camera->capture(image)) {
            i++;
            cv::Mat ocvImage;
            OpenCVUtility::cameraSampleToOcvMat(image, ocvImage, true);
            if (ocvImage.channels() != 1) {
                cv::cvtColor(ocvImage, ocvImage, CV_BGR2GRAY);
            }
            cv::imwrite(FLAGS_outputFolder + "/" + "Image_" + std::to_string(i) + ".pgm", ocvImage);
        }
    }

    return 0;
}
