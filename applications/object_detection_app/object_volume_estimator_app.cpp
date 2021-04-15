#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/util/json_util.h"
#include "packages/calibration/include/kannala_brandt_distortion_model.h"
#include "packages/calibration/include/kb4_image_undistortion.h"
#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/hal/proto/camera_id.pb.h"
#include "packages/machine_learning/include/object_detector.h"
#include "packages/machine_learning/include/object_volume_estimator_runner.h"

#include <fstream>

DEFINE_string(
    options, "", "object volume estimator options, see //packages/machine_learning/proto/object_volume_estimator_runner_options.proto");
DEFINE_string(system_calibration, "", "system calibration file");
DEFINE_string(cameraid, "", "FrontFisheye or RearFisheye, the camera where the object detection runs over.");

///
/// \brief Load KB4 Model
///
calibration::KannalaBrandtRadialDistortionModel4<double> loadKb4Model(const hal::CameraId& targetCameraId);

///
/// \brief Load fisheye camera intrinsic parameters
///
calibration::CameraIntrinsicCalibration loadCameraIntrinsicCalibration(const hal::CameraId& targetCameraId);

///
/// \brief Running object detection from a single camera stream
///
int main(int argc, char** argv) {

    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    gflags::ParseCommandLineFlags(&argc, &argv, false);
    FLAGS_logtostderr = true;

    if (FLAGS_system_calibration.empty()) {
        throw std::runtime_error("FLAGS_system_calibration is required.");
    }
    if (FLAGS_cameraid.empty()) {
        throw std::runtime_error("FLAGS_cameraid is required.");
    } else if (FLAGS_cameraid.compare("FrontFisheye") != 0 && FLAGS_cameraid.compare("RearFisheye") != 0) {
        throw std::runtime_error("FLAGS_cameraid should be either one of FrontFisheye or RearFisheye.");
    }
    hal::CameraId targetCameraId;
    if (FLAGS_cameraid.compare("FrontFisheye") == 0) {
        targetCameraId = hal::CameraId::FrontFisheye;
    } else if (FLAGS_cameraid.compare("RearFisheye") == 0) {
        targetCameraId = hal::CameraId::RearFisheye;
    }
    std::unique_ptr<ml::ObjectVolumeEstimatorRunner> estimator;
    if (!FLAGS_options.empty()) {
        estimator.reset(new ml::ObjectVolumeEstimatorRunner(loadKb4Model(targetCameraId), FLAGS_options));
    } else {
        estimator.reset(new ml::ObjectVolumeEstimatorRunner(loadKb4Model(targetCameraId)));
    }

    estimator->start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

calibration::KannalaBrandtRadialDistortionModel4<double> loadKb4Model(const hal::CameraId& targetCameraId) {
    Eigen::Matrix<double, 3, 3> K;
    Eigen::Matrix<double, 4, 1> coef;
    auto calibration = loadCameraIntrinsicCalibration(targetCameraId);
    K.row(0) << calibration.scaledfocallengthx(), 0, calibration.opticalcenterx();
    K.row(1) << 0, calibration.scaledfocallengthy(), calibration.opticalcentery();
    K.row(2) << 0, 0, 1;
    coef << calibration.kannalabrandt().radialdistortioncoefficientk(0), calibration.kannalabrandt().radialdistortioncoefficientk(1),
        calibration.kannalabrandt().radialdistortioncoefficientk(2), calibration.kannalabrandt().radialdistortioncoefficientk(3);
    return calibration::KannalaBrandtRadialDistortionModel4<double>(K, coef, 10, 1e-10);
}

calibration::CameraIntrinsicCalibration loadCameraIntrinsicCalibration(const hal::CameraId& targetCameraId) {
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
        if (parseSuccess && detectedCameraId == targetCameraId) {
            foundFisheyeIntrinsic = true;
            fisheyeIntrinsic = systemCalibration.cameraintrinsiccalibration(i);
        }
        CHECK(foundFisheyeIntrinsic) << "The system calibration does not have intrinsic parameters for the fisheye lens.";
    }
    CHECK(foundFisheyeIntrinsic) << "The fisheye lens is not found.";
    return fisheyeIntrinsic;
}