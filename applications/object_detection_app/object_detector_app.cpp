#include "packages/machine_learning/include/object_detector.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/util/json_util.h"
#include "packages/calibration/include/kannala_brandt_distortion_model.h"
#include "packages/calibration/include/kb4_image_undistortion.h"
#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/hal/proto/camera_id.pb.h"
#include "packages/machine_learning/include/object_detection_runner.h"

#include <fstream>

DEFINE_string(model, "", "TensorFlow detection model.");
DEFINE_string(options, "", "object detection runner options, see //packages/machine_learning/proto/object_detection_runner_options.proto");
DEFINE_string(system_calibration, "", "system calibration file");

///
/// \brief Load fisheye camera intrinsic parameters
calibration::CameraIntrinsicCalibration loadCameraIntrinsicCalibration();

///
/// \brief Running object detection from a single camera stream
///
int main(int argc, char** argv) {

    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    gflags::ParseCommandLineFlags(&argc, &argv, false);
    FLAGS_logtostderr = true;

    if (FLAGS_model.empty()) {
        throw std::runtime_error("FLAGS_model is required.");
    }

    if (FLAGS_system_calibration.empty()) {
        throw std::runtime_error("FLAGS_system_calibration is required.");
    }

    std::unique_ptr<ml::ObjectDetectionRunner> detectionRunner;
    if (!FLAGS_options.empty()) {
        detectionRunner.reset(new ml::ObjectDetectionRunner(FLAGS_model, loadCameraIntrinsicCalibration(), FLAGS_options));
    } else {
        detectionRunner.reset(new ml::ObjectDetectionRunner(FLAGS_model, loadCameraIntrinsicCalibration()));
    }

    detectionRunner->start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

calibration::CameraIntrinsicCalibration loadCameraIntrinsicCalibration() {
    ///// load calibration
    calibration::CameraIntrinsicCalibration fisheyeIntrinsic;
    bool foundFisheyeIntrinsic = false;
    calibration::SystemCalibration systemCalibration;

    std::ifstream file(FLAGS_system_calibration);
    std::stringstream buffer;
    buffer << file.rdbuf();
    if (buffer.str().size() == 0) {
        LOG(ERROR) << "Empty system calibration file";
        throw std::runtime_error("Empty system calibration file");
    }
    google::protobuf::util::JsonStringToMessage(buffer.str(), &systemCalibration);

    for (int i = 0; i < systemCalibration.cameraintrinsiccalibration().size(); i++) {
        hal::CameraId detectedCameraId;
        bool parseSuccess
            = hal::CameraId_Parse(systemCalibration.cameraintrinsiccalibration(i).cameraundercalibration().name(), &detectedCameraId);
        if (parseSuccess && detectedCameraId == hal::CameraId::FrontFisheye) {
            foundFisheyeIntrinsic = true;
            fisheyeIntrinsic = systemCalibration.cameraintrinsiccalibration(i);
        }
        CHECK(foundFisheyeIntrinsic) << "The system calibration does not have intrinsic parameters for the fisheye lens.";
    }
    return fisheyeIntrinsic;
}