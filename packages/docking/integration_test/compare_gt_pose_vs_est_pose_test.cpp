#include "gflags/gflags.h"

#include "Eigen/Eigen"
#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/docking/apriltag_fiducial_pose_source.h"
#include "packages/docking/ground_truth_fiducial_pose_source.h"
#include "packages/planning/utils.h"
#include "thirdparty/Sophus/sophus/se3.hpp"

using namespace docking;

DEFINE_string(system_calibration, "", "System calibration");
DEFINE_string(apriltag_side_length, "", "The apriltag side length in meter.");
DEFINE_string(video_port, "", "The port number where the camera stream comes in.");
DEFINE_string(fiducial_pose_port, "", "The port number where the fiducial poses stream comes in.");
DEFINE_string(angle_thres_in_deg, "", "The threshold on angle to check the ground-truth vs estimated.");
DEFINE_string(dist_thres_in_meter, "", "The threshold on distance to check the ground-truth vs estimated.");

///
/// The manual test to compare ground-truth poses vs detected poses
/// when the unity simulator (optionally with hald) is streaming out both images and apriltag poses
///
/// return code:
///   0 : success
///   1 : configuration missing
///   2 : detected poses are not consistent with ground-truth poses
///
int main(int argc, char** argv) {
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    ////////////////////////////////////
    /// Configuration
    ////////////////////////////////////

    /// required
    if (FLAGS_system_calibration.empty()) {
        LOG(INFO) << "System calibration is missing";
        return 1; // System calibration is missing
    }

    if (FLAGS_apriltag_side_length.empty()) {
        LOG(INFO) << "Apriltag side length is missing";
        return 1; // System calibration is missing
    }

    /// optional
    int video_port = 5556;
    if (!FLAGS_video_port.empty()) {
        video_port = std::stoi(FLAGS_video_port);
    }

    int fiducial_pose_port = 7601;
    if (!FLAGS_fiducial_pose_port.empty()) {
        fiducial_pose_port = std::stoi(FLAGS_fiducial_pose_port);
    }

    double dist_thres_in_meter = 0.05;
    if (!FLAGS_dist_thres_in_meter.empty()) {
        dist_thres_in_meter = std::stof(FLAGS_dist_thres_in_meter);
    }

    double angle_thres_in_deg = 5;
    if (!FLAGS_angle_thres_in_deg.empty()) {
        angle_thres_in_deg = std::stof(FLAGS_angle_thres_in_deg);
    }

    ////////////////////////////////////
    // Main
    ////////////////////////////////////
    const std::string fiducialPosesAddr("tcp://localhost:" + std::to_string(fiducial_pose_port));
    const std::string fiducialPosesTopic("fiducial_poses");
    GroundTruthFiducialPoseSource gSub(fiducialPosesAddr, fiducialPosesTopic);

    const std::string cameraStreamAddr("tcp://localhost:" + std::to_string(video_port));
    std::string cameraStreamTopic("camera");

    calibration::SystemCalibration calibration;
    planning::loadOptions(FLAGS_system_calibration, calibration, planning::SerializationType::JSON);

    calibration::CameraIntrinsicCalibration cameraIntrinsicCalibration = calibration.cameraintrinsiccalibration(0);

    perception::AprilTagConfig aprilTagConfig;
    aprilTagConfig.set_apriltagfamily(perception::AprilTagFamily::AprilTag36h11);
    aprilTagConfig.set_border(1);
    aprilTagConfig.set_sidelengthinmeters(std::stof(FLAGS_apriltag_side_length));

    perception::AprilTagDetectorOptions aprilTagDetectorOptions;
    aprilTagDetectorOptions.set_debug(false);
    aprilTagDetectorOptions.set_nthreads(1);
    aprilTagDetectorOptions.set_quaddecimate(4.0);
    aprilTagDetectorOptions.set_quadsigma(0.0);
    aprilTagDetectorOptions.set_refinedecode(0);
    aprilTagDetectorOptions.set_refineedges(0);
    aprilTagDetectorOptions.set_refinepose(0);

    ApriltagFiducialPoseSource dSub(
        cameraStreamAddr, cameraStreamTopic, cameraIntrinsicCalibration, aprilTagConfig, aprilTagDetectorOptions);

    docking::DockingStationList stationList;
    std::ifstream stationFile("/tmp/zippy_simulator_station_list.json");
    std::stringstream stationJsonBuffer;
    stationJsonBuffer << stationFile.rdbuf();
    google::protobuf::util::JsonStringToMessage(stationJsonBuffer.str(), &stationList);

    int nDetectedPoses = 0;

    while (nDetectedPoses < 1000) {
        for (int i = 0; i < (int)stationList.docking_stations_size(); i++) {
            std::vector<calibration::CoordinateTransformation> gPoses;
            gSub.readPoses(gPoses, stationList.docking_stations(i));

            std::vector<calibration::CoordinateTransformation> dPoses;
            dSub.readPoses(dPoses, stationList.docking_stations(i));

            for (int j = 0; j < (int)dPoses.size(); j++) {
                LOG(INFO) << dPoses[j].sourcecoordinateframe().device().name();

                for (int k = 0; k < (int)gPoses.size(); k++) {
                    if (dPoses[j].sourcecoordinateframe().device().name() == gPoses[k].sourcecoordinateframe().device().name()) {
                        LOG(INFO) << " ..  Found the ground truth info.";
                        LOG(INFO) << " ..  .. GT.r : " << gPoses[k].rodriguesrotationx() << ", " << gPoses[k].rodriguesrotationy() << ", "
                                  << gPoses[k].rodriguesrotationz();
                        LOG(INFO) << " ..  .. GT.t : " << gPoses[k].translationx() << ", " << gPoses[k].translationy() << ", "
                                  << gPoses[k].translationz();
                        LOG(INFO) << " ..  .. EST.r : " << dPoses[j].rodriguesrotationx() << ", " << dPoses[j].rodriguesrotationy() << ", "
                                  << dPoses[j].rodriguesrotationz();
                        LOG(INFO) << " ..  .. EST.t : " << dPoses[j].translationx() << ", " << dPoses[j].translationy() << ", "
                                  << dPoses[j].translationz();
                        nDetectedPoses++;

                        double gAngle = std::sqrt(gPoses[k].rodriguesrotationx() * gPoses[k].rodriguesrotationx()
                            + gPoses[k].rodriguesrotationy() * gPoses[k].rodriguesrotationy()
                            + gPoses[k].rodriguesrotationz() * gPoses[k].rodriguesrotationz());
                        Sophus::SO3d gPoseSO3(Eigen::Quaternion<double>(Eigen::AngleAxisd(
                            gAngle, Eigen::Vector3d(gPoses[k].rodriguesrotationx() / gAngle, gPoses[k].rodriguesrotationy() / gAngle,
                                        gPoses[k].rodriguesrotationz() / gAngle))));
                        Sophus::SE3d gPose(
                            gPoseSO3, Sophus::SE3d::Point(gPoses[k].translationx(), gPoses[k].translationy(), gPoses[k].translationz()));

                        double dAngle = std::sqrt(dPoses[j].rodriguesrotationx() * dPoses[j].rodriguesrotationx()
                            + dPoses[j].rodriguesrotationy() * dPoses[j].rodriguesrotationy()
                            + dPoses[j].rodriguesrotationz() * dPoses[j].rodriguesrotationz());
                        Sophus::SO3d dPoseSO3(Eigen::Quaternion<double>(Eigen::AngleAxisd(
                            dAngle, Eigen::Vector3d(dPoses[j].rodriguesrotationx() / dAngle, dPoses[j].rodriguesrotationy() / dAngle,
                                        dPoses[j].rodriguesrotationz() / dAngle))));
                        Sophus::SE3d dPose(
                            dPoseSO3, Sophus::SE3d::Point(dPoses[j].translationx(), dPoses[j].translationy(), dPoses[j].translationz()));

                        Sophus::SE3d diffPose = dPose.inverse() * gPose;

                        /// Convert into axis/angle
                        Eigen::AngleAxisd diffAngle(diffPose.unit_quaternion());
                        double diffDist = diffPose.translation().norm();

                        if (diffDist > dist_thres_in_meter || diffAngle.angle() > angle_thres_in_deg / 180 * M_PI) {
                            LOG(INFO) << "The detected pose is different from the ground truth.";
                            return 2;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
