#include "glog/logging.h"
#include "gtest/gtest.h"
#include <limits>

#include "packages/docking/inverse_kinematic_controller_helper.h"

using namespace docking;

///
/// Regression test with the manually defined Vehicle setup.
/// Especially, the following setup is to have a rotation pitch at position (0,0,-1)
/// and the anchor point from the pitch-axis by (1,0,0). As a result, the location of anchor point is
/// supposed to be (1,0,-1). For simplicity, no roation applied.
///
TEST(getSE3ChassisCenterWrtVehicleDockAnchor, regression_preset_1) {

    Sophus::SE3<float> se3OwrtF;
    Sophus::SE3<float> se3Perturb;
    se3Perturb.translation() = Eigen::Matrix<float, 3, 1>(0, 0, 0);
    se3Perturb.setQuaternion(Eigen::Quaternion<float>(1, 0, 0, 0));
    float leftRailPerturb = 0;
    float rightRailPerturb = 0;
    float bearingAnglePerturb = 0;
    VcuIkTelemetryState telemetryState;
    telemetryState.leftServoAngle = 0;
    telemetryState.rightServoAngle = 0;
    telemetryState.leftServoHeight = 1; // left&right heights=1 <-> z=-1
    telemetryState.rightServoHeight = 1;
    Sophus::SE3<float> se3JpwrtF;
    se3JpwrtF.translation() = Eigen::Matrix<float, 3, 1>(1, 0, 0);
    se3JpwrtF.setQuaternion(Eigen::Quaternion<float>(1, 0, 0, 0));
    float distBetweenWheel = 1;
    getSE3ChassisCenterWrtVehicleDockAnchor(
        se3OwrtF, se3Perturb, leftRailPerturb, rightRailPerturb, bearingAnglePerturb, telemetryState, se3JpwrtF, distBetweenWheel);

    EXPECT_NEAR(se3OwrtF.unit_quaternion().coeffs().data()[3], 1, std::numeric_limits<float>::epsilon());
    EXPECT_NEAR(se3OwrtF.translation().data()[0], 1, std::numeric_limits<float>::epsilon());
    EXPECT_NEAR(se3OwrtF.translation().data()[1], 0, std::numeric_limits<float>::epsilon());
    EXPECT_NEAR(se3OwrtF.translation().data()[2], -1, std::numeric_limits<float>::epsilon());
}

///
/// Regression test with the manually defined Vehicle setup.
/// Especially, the following setup is to have a rotation joint at position (0,0,-1)
/// and the anchor point from the pitch-axis by (1,0,0), rotated by 90 degree in pitch.
/// As a result, the location of anchor point is (0,0,-2)
///
TEST(getSE3ChassisCenterWrtVehicleDockAnchor, regression_preset_2) {

    Sophus::SE3<float> se3OwrtF;
    Sophus::SE3<float> se3Perturb;
    se3Perturb.translation() = Eigen::Matrix<float, 3, 1>(0, 0, 0);
    se3Perturb.setQuaternion(Eigen::Quaternion<float>(1, 0, 0, 0));
    float leftRailPerturb = 0;
    float rightRailPerturb = 0;
    float bearingAnglePerturb = 0;
    VcuIkTelemetryState telemetryState;
    telemetryState.leftServoAngle = M_PI / 2;
    telemetryState.rightServoAngle = M_PI / 2;
    telemetryState.leftServoHeight = 1; // left&right heights=1 <-> z=-1
    telemetryState.rightServoHeight = 1;
    Sophus::SE3<float> se3JpwrtF;
    se3JpwrtF.translation() = Eigen::Matrix<float, 3, 1>(1, 0, 0);
    se3JpwrtF.setQuaternion(Eigen::Quaternion<float>(1, 0, 0, 0));
    float distBetweenWheel = 1;
    getSE3ChassisCenterWrtVehicleDockAnchor(
        se3OwrtF, se3Perturb, leftRailPerturb, rightRailPerturb, bearingAnglePerturb, telemetryState, se3JpwrtF, distBetweenWheel);

    EXPECT_NEAR(se3OwrtF.translation().data()[0], 0, std::numeric_limits<float>::epsilon());
    EXPECT_NEAR(se3OwrtF.translation().data()[1], 0, std::numeric_limits<float>::epsilon());
    EXPECT_NEAR(se3OwrtF.translation().data()[2], -2, std::numeric_limits<float>::epsilon());
}
