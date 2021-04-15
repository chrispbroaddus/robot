#pragma once

#include "packages/calibration/include/kannala_brandt_distortion_model.h"
#include "packages/calibration/include/kb4_image_undistortion.h"
#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/machine_learning/include/object_detector.h"
#include "packages/machine_learning/proto/object_detection_runner_options.pb.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "packages/net/include/zmq_topic_sub.h"

#include <atomic>
#include <thread>

namespace ml {

///
/// \brief Wrapper class running object detection on a camera stream from a single RGB camera.
///
class ObjectDetectionRunner {

public:
    ///
    /// \param modelPath The file path to the object detection mode, more details at //packages/machine_learning/models/BUILD.
    /// \param calibration The system calibration.
    /// \param options The file path where program option can be loaded.
    ///
    ObjectDetectionRunner(
        const std::string& modelPath, const calibration::CameraIntrinsicCalibration& calibration, const std::string& options);

    ///
    /// \param modelPath The file path to the object detection mode, more details at //packages/machine_learning/models/BUILD.
    /// \param calibration The system calibration.
    ///
    ObjectDetectionRunner(const std::string& modelPath, const calibration::CameraIntrinsicCalibration& calibration);

    ObjectDetectionRunner(const ObjectDetectionRunner&&) = delete;
    ObjectDetectionRunner& operator=(ObjectDetectionRunner&) = delete;
    ObjectDetectionRunner& operator=(ObjectDetectionRunner&&) = delete;

    ~ObjectDetectionRunner();

    ///
    /// \brief Start detection on a separate thread.
    ///
    void start();

private:
    ///
    /// \brief The main detecting function called from start()
    ///
    void detect(std::atomic_bool& stop);

    ///
    /// \brief If m_config.undistort()==true, re-distort the obunding boxes to be consistent with origianl distorted images.
    ///
    void distortBoundingBoxes(perception::Detection& detection);

    std::atomic_bool m_stop;
    ObjectDetectionRunnerOptions m_config;
    Eigen::Matrix<double, 3, 3> m_K;
    Eigen::Matrix<double, 4, 1> m_coef;
    std::unique_ptr<calibration::Kb4ImageUndistortion<double> > m_undistortion;
    std::unique_ptr<calibration::KannalaBrandtRadialDistortionModel4<double> > m_kb4Model;
    std::unique_ptr<ml::ObjectDetector> m_objectDetector;
    std::unique_ptr<std::thread> m_thread;
    std::string m_topic;
};
}
