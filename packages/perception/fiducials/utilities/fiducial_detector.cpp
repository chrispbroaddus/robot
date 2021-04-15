#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/hal/include/drivers/cameras/flycapture/flycapture_driver_factory.h"
#include "packages/perception/fiducials/apriltag_detector.h"

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/util/json_util.h"

#include <atomic>
#include <fstream>
#include <thread>

DEFINE_string(systemCalibrationFile, "", "system calibration file");
DEFINE_string(cameraSerialNumber, "", "camera serial number");

std::atomic_bool run;
void sig_handler(int /* s */) { run = false; }

int main(int argc, char* argv[]) {

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = sig_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    google::InitGoogleLogging(argv[0]);

    gflags::SetUsageMessage("Fiducial Detector");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    if (FLAGS_systemCalibrationFile.empty()) {
        gflags::ShowUsageWithFlagsRestrict(argv[0], "packages/perception/fiducials/utilities/fiducial_detector.cpp");
        return 0;
    }

    uint64_t serialNumber = std::stoul(FLAGS_cameraSerialNumber);
    FlyCapture2::GrabMode grabMode = FlyCapture2::DROP_FRAMES;
    uint32_t captureTimeoutInMilliseconds = 10;
    FlyCapture2::Mode format7Mode = FlyCapture2::MODE_0;
    FlyCapture2::PixelFormat pixelFormat = FlyCapture2::PIXEL_FORMAT_MONO8;
    uint32_t width = 2448;
    uint32_t height = 2048;
    uint32_t left = 0;
    uint32_t top = 0;
    uint32_t numBuffers = 10;
    float fps = 30;
    bool externalTrigger = false;
    uint32_t AERoILeft = 0;
    uint32_t AERoITop = 0;
    uint32_t AERoIWidth = 2448;
    uint32_t AERoIHeight = 2048;
    hal::FlycaptureDriver flycaptureDriver(serialNumber, grabMode, format7Mode, pixelFormat, left, top, width, height, fps, numBuffers,
        captureTimeoutInMilliseconds, externalTrigger, AERoILeft, AERoITop, AERoIWidth, AERoIHeight);

    LOG(INFO) << "Created camera";

    perception::AprilTagDetectorOptions aprilTagDetectorOptions = perception::loadDefaultAprilTagDetectorOptions();
    perception::AprilTagConfig aprilTagConfig = perception::loadDefaultAprilTagConfig();

    calibration::SystemCalibration systemCalibration;
    std::ifstream file(FLAGS_systemCalibrationFile);
    std::stringstream buffer;
    buffer << file.rdbuf();
    google::protobuf::util::JsonStringToMessage(buffer.str(), &systemCalibration);

    calibration::CameraIntrinsicCalibration cameraIntrinsicCalibration;
    int index;
    for (index = 0; index < systemCalibration.cameraintrinsiccalibration_size(); index++) {
        if (systemCalibration.cameraintrinsiccalibration(index).cameraundercalibration().serialnumber() == serialNumber) {
            cameraIntrinsicCalibration = systemCalibration.cameraintrinsiccalibration(index);
            break;
        }
    }
    if (index == systemCalibration.cameraintrinsiccalibration_size()) {
        LOG(ERROR) << "Fiducial Detector: No calibration found for camera with serialNumber: " << serialNumber;
        throw std::runtime_error("Fiducial Detector: No calibration found for camera");
    }

    std::shared_ptr<perception::FiducialDetectorInterface> fiducialDetector
        = std::make_shared<perception::AprilTagDetector>(cameraIntrinsicCalibration, aprilTagConfig, aprilTagDetectorOptions);

    run = true;
    while (run) {
        hal::CameraSample cameraSample;
        if (flycaptureDriver.capture(cameraSample)) {
            std::vector<calibration::CoordinateTransformation> detectedPoses;
            if (fiducialDetector->estimatePose(cameraSample, detectedPoses)) {
                for (uint32_t i = 0; i < detectedPoses.size(); i++) {
                    LOG(INFO) << "ID: " << detectedPoses[i].sourcecoordinateframe().device().serialnumber()
                              << " Translation x: " << detectedPoses[i].translationx() << " y: " << detectedPoses[i].translationy()
                              << " z: " << detectedPoses[i].translationz();
                    double norm = std::sqrt(std::pow(detectedPoses[i].rodriguesrotationx(), 2)
                        + std::pow(detectedPoses[i].rodriguesrotationy(), 2) + std::pow(detectedPoses[i].rodriguesrotationz(), 2));
                    double angleInDegrees = norm * 180.0 / M_PI;
                    LOG(INFO) << "Rotation Angle: " << angleInDegrees << " Axis x: " << detectedPoses[i].rodriguesrotationx() / norm
                              << " y: " << detectedPoses[i].rodriguesrotationy() / norm
                              << " z: " << detectedPoses[i].rodriguesrotationz() / norm;
                }
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    gflags::ShutDownCommandLineFlags();
    return 0;
}
