#include "apriltag_fiducial_pose_source.h"
#include "Eigen/Eigen"
#include "glog/logging.h"
#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/core/include/chrono.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/math/geometry/quaternion.h"
#include "packages/math/geometry/se3.h"
#include "thirdparty/Sophus/sophus/se3.hpp"

namespace docking {

ApriltagFiducialPoseSource::ApriltagFiducialPoseSource(const std::string& cameraSampleSubscribeAddress, const std::string& topic,
    const calibration::CameraIntrinsicCalibration& cameraIntrinsicCalibration, const perception::AprilTagConfig& aprilTagConfig,
    const perception::AprilTagDetectorOptions& aprilTagDetectorOptions)
    : m_detector(cameraIntrinsicCalibration, aprilTagConfig, aprilTagDetectorOptions)
    , m_stop(false)
    , m_thread(&ApriltagFiducialPoseSource::listen, this, cameraSampleSubscribeAddress, topic, std::ref(m_stop)) {}

ApriltagFiducialPoseSource::~ApriltagFiducialPoseSource() {
    m_stop = true;
    m_thread.join();
}

bool ApriltagFiducialPoseSource::readPoses(
    std::vector<calibration::CoordinateTransformation>& retPoses, const docking::DockingStation& dockingStation) {

    std::lock_guard<std::mutex> guard(m_mutex);

    // todo : timestamp is wrong
    if (m_sample.has_image()) // && gpsTimestamp - m_sample.systemtimestamp().nanos() < 100 * 1e6) // 100 ms
    {
        auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds().count();
        VLOG(2) << __PRETTY_FUNCTION__ << " ... gpsTimestamp : " << gpsTimestamp;
        VLOG(2) << __PRETTY_FUNCTION__ << " ... m_sample.systemtimestamp() : " << m_sample.systemtimestamp().nanos();
        VLOG(2) << __PRETTY_FUNCTION__ << " ... timediff : " << gpsTimestamp - m_sample.systemtimestamp().nanos();

        std::vector<calibration::CoordinateTransformation> poses;
        m_detector.estimatePose(m_sample, poses);

        VLOG(1) << __FUNCTION__ << " ... # of detected apriltags : " << poses.size();
        for (int i = 0; i < (int)poses.size(); i++) {

            bool foundFiducialOnDockingStation = false;

            // update the pose from the transformation
            calibration::CoordinateTransformation pose = poses[i];

            for (int j = 0; j < dockingStation.fiducials_size(); j++) {
                if (pose.sourcecoordinateframe().device().name()
                        == dockingStation.fiducials(j).transformation().sourcecoordinateframe().device().name()
                    && pose.sourcecoordinateframe().device().serialnumber()
                        == dockingStation.fiducials(j).transformation().sourcecoordinateframe().device().serialnumber()) {
                    VLOG(2) << __FUNCTION__ << " ... ... source : " << pose.sourcecoordinateframe().device().name();
                    VLOG(2) << __FUNCTION__
                            << " ... ... target : " << dockingStation.fiducials(j).transformation().sourcecoordinateframe().device().name();
                    VLOG(2) << __FUNCTION__ << " ... ... pose.translationx() : " << pose.translationx();
                    VLOG(2) << __FUNCTION__ << " ... ... pose.translationy() : " << pose.translationy();
                    VLOG(2) << __FUNCTION__ << " ... ... pose.translationz() : " << pose.translationz();

                    foundFiducialOnDockingStation = true;
                }
            }

            if (foundFiducialOnDockingStation) {
                calibration::CoordinateTransformation rawPose = poses[i];

                Sophus::SE3f sophusPose = geometry::getSophusSE3<float>(rawPose);

                // Rotate the coordinate:
                //   sophusPose : Pose_OpenCVCameraCoord_wrt_ApriltagInXYPlaneCoord
                //   into
                //   Pose_ZippyTorsoCoord_wrt_ApriltagInYZPlaneCoord
                //   by multiplying:
                //   t3 * t2 : Pose_ZippyTorsoCoord_wrt_OpenCVCameraCoord
                //   t1 : Pose_ApriltagInYZPlaneCoord_wrt_ApriltagInXYPlaneCoord
                Sophus::SO3f t1q(Eigen::Quaternionf(Eigen::AngleAxisf(-0.5 * M_PI, Eigen::Vector3f(0.f, 1.f, 0.f))));
                Sophus::SE3f t1(t1q, Sophus::SE3f::Point(0, 0, 0));
                Sophus::SO3f t2q(Eigen::Quaternionf(Eigen::AngleAxisf(0.5 * M_PI, Eigen::Vector3f(0.f, 1.f, 0.f))));
                Sophus::SE3f t2(t2q, Sophus::SE3f::Point(0, 0, 0));
                Sophus::SO3f t3q(Eigen::Quaternionf(Eigen::AngleAxisf(0.5 * M_PI, Eigen::Vector3f(1.f, 0.f, 0.f))));
                Sophus::SE3f t3(t3q, Sophus::SE3f::Point(0, 0, 0));
                Sophus::SE3f newPose = t3 * t2 * sophusPose * t1;
                Eigen::AngleAxisf newPoseAngleAxis(newPose.unit_quaternion());

                pose.set_translationx(newPose.translation().coeff(0));
                pose.set_translationy(newPose.translation().coeff(1));
                pose.set_translationz(newPose.translation().coeff(2));
                pose.set_rodriguesrotationx(newPoseAngleAxis.axis().coeff(0) * newPoseAngleAxis.angle());
                pose.set_rodriguesrotationy(newPoseAngleAxis.axis().coeff(1) * newPoseAngleAxis.angle());
                pose.set_rodriguesrotationz(newPoseAngleAxis.axis().coeff(2) * newPoseAngleAxis.angle());

                retPoses.push_back(pose);
            }
        }
        VLOG(1) << __FUNCTION__;
        return retPoses.size() > 0;
    } else {
        return false;
    }
}

void ApriltagFiducialPoseSource::listen(const std::string& addr, const std::string& topic, std::atomic_bool& stop) {

    m_sample.clear_image();

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<hal::CameraSample> subscriber(context, addr, topic, 1);

    while (!stop) {

        if (subscriber.poll(std::chrono::milliseconds(1))) {
            hal::CameraSample sample;
            if (subscriber.recv(sample)) {
                std::lock_guard<std::mutex> guard(m_mutex);
                std::swap(m_sample, sample);
            } // if (subscriber.recv())

        } // if (subscriber.poll())

    } // while(!stop)
}
}
