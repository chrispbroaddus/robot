#pragma once

#include "Eigen/Eigen"
#include "thirdparty/Sophus/sophus/se3.hpp"
#include <cmath>

namespace unity_plugins {

///
/// The following conversions assume:
///  * Unity Vehicle Coordinate
///   -- left-handed, +z : vehicle-forward, +y : opposite-to-gravity, +x : vehicle-rightward
///  * Zippy Vehicle Coordinate (https://zippy.quip.com/cabrABBrDn2C)
///   -- right-handed, +x : vehicle-forward, +z : gravity-direction, +y : vehicle-rightward
///
template <typename T>
inline void convertUnityVehicleToZippyVehicleCoordinate(
    T& rxp, T& ryp, T& rzp, T& txp, T& typ, T& tzp, T rx, T ry, T rz, T tx, T ty, T tz) {
    rxp = -rz;
    ryp = -rx;
    rzp = ry;

    txp = tz;
    typ = tx;
    tzp = -ty;
}

///
/// The following conversions assume:
///  * Unity Vehicle Coordinate
///   -- left-handed, +z : vehicle-forward, +y : opposite-to-gravity, +x : vehicle-rightward
///  * Zippy Vehicle Coordinate (https://zippy.quip.com/cabrABBrDn2C)
///   -- right-handed, +x : vehicle-forward, +z : gravity-direction, +y : vehicle-rightward
///
template <typename T>
inline void convertZippyVehicleToUnityVehicleCoordinate(
    T& rxp, T& ryp, T& rzp, T& txp, T& typ, T& tzp, T rx, T ry, T rz, T tx, T ty, T tz) {
    rxp = -ry;
    ryp = rz;
    rzp = -rx;

    txp = ty;
    typ = -tz;
    tzp = tx;
}

///
/// The following conversions assume:
///  * Unity Apriltag Coordinate
///   -- left-handed, Apriltag is on the +x : left, +z : downward plane
///  * Apriltag Coordinate used for docking
///   -- right-handed, Apriltag is on the +y : right, +z : downward plane
///
template <typename T>
inline void convertUnityAprilTagToDockingAprilTagCoordinate(
    T& rxpp, T& rypp, T& rzpp, T& txpp, T& typp, T& tzpp, T rx, T ry, T rz, T tx, T ty, T tz) {
    T rxp = -rz;
    T ryp = -rx;
    T rzp = ry;

    T txp = tz;
    T typ = tx;
    T tzp = -ty;

    const float angle = std::sqrt(rxp * rxp + ryp * ryp + rzp * rzp);
    Sophus::SO3f poseq;
    if (std::fabs(angle) < 1e-5) {
        poseq = Sophus::SO3f(Eigen::Quaternionf(Eigen::AngleAxisf(1e-7, Eigen::Vector3f(1.f, 0.f, 0.f))));
    } else {
        poseq = Sophus::SO3f(Eigen::Quaternionf(Eigen::AngleAxisf(angle, Eigen::Vector3f(rxp / angle, ryp / angle, rzp / angle))));
    }

    Sophus::SE3f pose(poseq, Sophus::SE3f::Point(txp, typ, tzp));
    Sophus::SO3f t1q(Eigen::Quaternionf(Eigen::AngleAxisf(0.5 * M_PI, Eigen::Vector3f(0.f, 1.f, 0.f))));
    Sophus::SE3f t1(t1q, Sophus::SE3f::Point(0, 0, 0));
    Sophus::SO3f t2q(Eigen::Quaternionf(Eigen::AngleAxisf(M_PI, Eigen::Vector3f(0.f, 0.f, 1.f))));
    Sophus::SE3f t2(t2q, Sophus::SE3f::Point(0, 0, 0));
    Sophus::SE3f newPose = t2.inverse() * t1.inverse() * pose * t1 * t2;

    Eigen::AngleAxisf newPoseAngleAxis(newPose.unit_quaternion());
    rxpp = newPoseAngleAxis.axis().coeff(0) * newPoseAngleAxis.angle();
    rypp = newPoseAngleAxis.axis().coeff(1) * newPoseAngleAxis.angle();
    rzpp = newPoseAngleAxis.axis().coeff(2) * newPoseAngleAxis.angle();
    txpp = newPose.translation().coeff(0);
    typp = newPose.translation().coeff(1);
    tzpp = newPose.translation().coeff(2);
}
}
