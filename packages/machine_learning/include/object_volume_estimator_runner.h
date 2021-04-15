#pragma once

#include "packages/calibration/include/kannala_brandt_distortion_model.h"
#include "packages/calibration/include/kb4_image_undistortion.h"
#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/machine_learning/proto/object_volume_estimator_runner_options.pb.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "packages/net/include/zmq_topic_sub.h"

#include <atomic>
#include <thread>

namespace ml {

///
/// \brief Class estimating the volume of the detected object with the fused data from stereo depth cameras.
///
class ObjectVolumeEstimatorRunner {

public:
    ///
    /// \param calibration The system calibration.
    /// \param options The file path where program option can be loaded.
    ///
    ObjectVolumeEstimatorRunner(const calibration::KannalaBrandtRadialDistortionModel4<double>& kb4Model, const std::string& options);

    ///
    /// \param calibration The system calibration.
    ///
    ObjectVolumeEstimatorRunner(const calibration::KannalaBrandtRadialDistortionModel4<double>& kb4Model);

    ObjectVolumeEstimatorRunner(const ObjectVolumeEstimatorRunner&&) = delete;
    ObjectVolumeEstimatorRunner& operator=(ObjectVolumeEstimatorRunner&) = delete;
    ObjectVolumeEstimatorRunner& operator=(ObjectVolumeEstimatorRunner&&) = delete;

    ~ObjectVolumeEstimatorRunner();

    ///
    /// \brief Start volume estimator on a separate thread.
    ///
    void start();

private:
    ObjectVolumeEstimatorRunnerOptions m_config;

    const calibration::KannalaBrandtRadialDistortionModel4<double>& m_kb4Model;
    std::unique_ptr<std::thread> m_thread;
    std::atomic_bool m_stop;

    std::vector<double> m_depthMapOnImageSpace;
    double m_maxPointCloudDistance;
    Eigen::Matrix<double, 3, 3> m_rotationFisheyeWrtLeftStereo;
    Eigen::Matrix<double, 3, 1> m_translationFisheyeWrtLeftStereo;

    ///
    /// \brief The main estimation function called from start()
    ///
    void estimate(std::atomic_bool& stop);

    ///
    /// \brief Project point cloud into image space
    ///
    void projectPointCloudOnImageSpace(const hal::CameraSample& depthSample, const int rgbImageRows, const int rgbImageCols);
};
}
