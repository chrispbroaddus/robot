#pragma once

#include "glog/logging.h"
#include "thirdparty/Sophus/sophus/se3.hpp"
#include "vcu_ik_telemetry_state.h"

namespace docking {

using namespace Sophus;

///
/// \param se3OwrtF
/// \param se3Perturb
/// \param leftRailPerturb
/// \param rightRailPerturb
/// \param bearingAnglePerturb
/// \param leftServoAngle
/// \param rightServoAngle
/// \param leftServoHeight
/// \param rightServoHeight
/// \param se3JpwrtF
/// \param distBetweenWheel
/// @details See https://zippy.quip.com/2XpnASQU95k4 for more details.
///          Notations: O: Chassis Center, F : Vehicle's Docking Anchor Poin
///                     J: Pitch-Axis-Center-Coordinate without applying pitch rotation
///                     Jp: Pitch-Axis-Center-Coordinate after applying pitch rotation
template <typename T>
void getSE3ChassisCenterWrtVehicleDockAnchor(Sophus::SE3<T>& se3OwrtF, const Sophus::SE3<T> se3Perturb, const T leftRailPerturb,
    const T rightRailPerturb, const T bearingAnglePerturb, const VcuIkTelemetryState& telemetryState, const Sophus::SE3<T> se3JpwrtF,
    const T distBetweenWheel) {
    constexpr T pi(M_PI);

    CHECK(distBetweenWheel > 0);

    SE3<T> se3OwrtWh = se3Perturb;

    SE3<T> se3JwrtWh;
    T rightRailHeight = telemetryState.leftServoHeight + rightRailPerturb;
    T leftRailHeight = telemetryState.rightServoHeight + leftRailPerturb;

    VLOG(1) << __FUNCTION__ << " ... rightRailHeight : " << rightRailHeight;
    VLOG(1) << __FUNCTION__ << " ... leftRailHeight : " << leftRailHeight;

    if (rightRailHeight > leftRailHeight) {
        T phi = std::atan2(rightRailHeight - leftRailHeight, distBetweenWheel);
        CHECK(phi >= 0);
        CHECK(phi < pi / 2);

        T c = std::cos(phi / 2);
        T s = std::sin(phi / 2);
        se3JwrtWh.setQuaternion(Eigen::Quaternion<T>(c, s, 0, 0));
        se3JwrtWh.translation() = Eigen::Matrix<T, 3, 1>(0, 0, leftRailHeight + distBetweenWheel / 2.f * std::tan(phi));
    } else {
        T phi = std::atan((leftRailHeight - rightRailHeight) / distBetweenWheel);
        CHECK(phi >= 0);
        CHECK(phi < pi / 2);

        T c = std::cos(phi / 2);
        T s = std::sin(phi / 2);
        se3JwrtWh.setQuaternion(Eigen::Quaternion<T>(c, -s, 0, 0));
        se3JwrtWh.translation() = Eigen::Matrix<T, 3, 1>(0, 0, rightRailHeight + distBetweenWheel / 2.f * std::tan(phi));
    }

    SE3<T> se3WhwrtJ = se3JwrtWh.inverse();
    SE3<T> se3JwrtJp;
    T pLeftServoAngle = telemetryState.leftServoAngle + bearingAnglePerturb;
    T c = std::cos(pLeftServoAngle / 2);
    T s = std::sin(pLeftServoAngle / 2);
    se3JwrtJp.setQuaternion(Eigen::Quaternion<T>(c, 0, s, 0));
    se3JwrtJp.translation() = Eigen::Matrix<T, 3, 1>(0, 0, 0);

    VLOG(1) << __FUNCTION__ << " ... se3JpwrtF : " << se3JpwrtF.matrix3x4();
    VLOG(1) << __FUNCTION__ << " ... se3JwrtJp : " << se3JwrtJp.matrix3x4();
    VLOG(1) << __FUNCTION__ << " ... se3WhwrtJ : " << se3WhwrtJ.matrix3x4();
    VLOG(1) << __FUNCTION__ << " ... se3OwrtWh : " << se3OwrtWh.matrix3x4();

    se3OwrtF = se3OwrtWh * se3WhwrtJ * se3JwrtJp * se3JpwrtF;
}

///
/// \param se3OwrtF
/// \param leftServoAngle
/// \param rightServoAngle
/// \param leftServoHeight
/// \param rightServoHeight
/// \param se3JpwrtF
/// \param distBetweenWheel
/// @details See https://zippy.quip.com/2XpnASQU95k4 for more details.
///          Notations: O: Chassis Center, F : Vehicle's Docking Anchor Poin
///                     J: Pitch-Axis-Center-Coordinate without applying pitch rotation
///                     Jp: Pitch-Axis-Center-Coordinate after applying pitch rotation
template <typename T>
void getSE3ChassisCenterWrtVehicleDockAnchor(
    Sophus::SE3<T>& se3OwrtF, const VcuIkTelemetryState& telemetryState, const Sophus::SE3<T> se3JpwrtF, const T distBetweenWheel) {
    Sophus::SE3<T> se3Perturb(Eigen::Quaternion<T>(1, 0, 0, 0), Eigen::Matrix<T, 3, 1>(0, 0, 0));
    T leftRailPerturb = 0;
    T rightRailPerturb = 0;
    T bearingAnglePerturb = 0;
    getSE3ChassisCenterWrtVehicleDockAnchor(
        se3OwrtF, se3Perturb, leftRailPerturb, rightRailPerturb, bearingAnglePerturb, telemetryState, se3JpwrtF, distBetweenWheel);
}

///
/// \brief The derivative of SE3 vector (cost) over scalar delta (one variable perturbation)
/// \param delta
/// \param diffInSe3
///
template <typename T> const Eigen::Matrix<T, 6, 1> divideSE3OverDelta(T delta, const Sophus::SE3<T>& diffInSe3) {
    constexpr T pi(M_PI);

    Eigen::Matrix<T, 6, 1> J;
    T theta = std::acos(diffInSe3.unit_quaternion().w()) * 2;
    T c = std::cos(theta / 2);
    T r0 = diffInSe3.unit_quaternion().x() / c;
    T r1 = diffInSe3.unit_quaternion().y() / c;
    T r2 = diffInSe3.unit_quaternion().z() / c;
    J(0, 0) = r0 < pi ? r0 : r0 - 2 * pi;
    J(1, 0) = r1 < pi ? r1 : r1 - 2 * pi;
    J(2, 0) = r2 < pi ? r2 : r2 - 2 * pi;
    J(3, 0) = diffInSe3.translation()[0];
    J(4, 0) = diffInSe3.translation()[1];
    J(5, 0) = diffInSe3.translation()[2];
    J /= delta;
    return J;
}

///
/// \param J
/// \param delta
/// \param se3OwrtF
/// \param se3Perturb
/// \param leftRailPerturb
/// \param rightRailPerturb
/// \param bearingAnglePerturb
/// \param leftServoAngle
/// \param rightServoAngle
/// \param leftServoHeight
/// \param rightServoHeight
/// \param se3JpwrtF
/// \param distBetweenWheel
/// @details See https://zippy.quip.com/2XpnASQU95k4 for more details.
///          Notations: O: Chassis Center, F : Vehicle's Docking Anchor Poin
///                     J: Pitch-Axis-Center-Coordinate without applying pitch rotation
///                     Jp: Pitch-Axis-Center-Coordinate after applying pitch rotation
template <typename T>
const Eigen::Matrix<T, 6, 1> getJacobianPerColumn(const T delta, const Sophus::SE3<T> se3OwrtF, const Sophus::SE3<T> se3Perturb,
    const T leftRailPerturb, const T rightRailPerturb, const T bearingAnglePerturb, const VcuIkTelemetryState telemetryState,
    const Sophus::SE3<T> se3JpwrtF, const T distBetweenWheel) {
    Sophus::SE3<T> se3OwrtFp;
    getSE3ChassisCenterWrtVehicleDockAnchor(
        se3OwrtFp, se3Perturb, leftRailPerturb, rightRailPerturb, bearingAnglePerturb, telemetryState, se3JpwrtF, distBetweenWheel);

    Sophus::SE3<T> diffInSe3 = se3OwrtFp * se3OwrtF.inverse();

    VLOG(1) << __PRETTY_FUNCTION__ << diffInSe3.matrix3x4();

    return divideSE3OverDelta(delta, diffInSe3);
}

enum class VehiclePerturbedParameter : uint16_t {
    ChassisXPosition,
    ChassisYPosition,
    ChassisRotationRadians,
    LeftRailHeight,
    RightRailHeight,
    RotatinPitch
};

///
/// \brief Update the perturbation of the state based on the index.
/// \param delta
/// \param se3Perturb
/// \param leftRailPerturb
/// \param rightRailPerturb
/// \param bearingAnglePerturb
/// \param parameter. 0: chassis-x-pos, 2: chassis-z-pos, 3: chassis-rot-radian,
///               6: left-rail-height, 7: right-rail-height, 8: pitch-rotating-radian
/// \return
template <typename T>
bool perturbState(T& delta, Sophus::SE3<T>& se3Perturb, T& leftRailPerturb, T& rightRailPerturb, T& bearingAnglePerturb,
    VehiclePerturbedParameter parameter) {

    switch (parameter) {
    case VehiclePerturbedParameter::ChassisXPosition:
        delta = T(0.01);
        se3Perturb.translation()(0, 0) += delta;
        break;

    case VehiclePerturbedParameter::ChassisYPosition:
        delta = T(0.01);
        se3Perturb.translation()(1, 0) += delta;
        break;

    case VehiclePerturbedParameter::ChassisRotationRadians: {
        delta = T(0.01);
        T c = std::cos(delta / 2);
        T s = std::cos(delta / 2);
        se3Perturb.setQuaternion(Eigen::Quaternion<T>(c, 0, 0, s));
    } break;

    case VehiclePerturbedParameter::LeftRailHeight:
        delta = T(0.01);
        leftRailPerturb += delta;
        break;

    case VehiclePerturbedParameter::RightRailHeight:
        delta = T(0.01);
        rightRailPerturb += delta;
        break;

    case VehiclePerturbedParameter::RotatinPitch:
        delta = T(3);
        bearingAnglePerturb -= delta;
        break;

    default:
        return false;
    }
    return true;
}
}