#pragma once

#include "Eigen/Eigen"
#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "thirdparty/Sophus/sophus/se3.hpp"
#include "thirdparty/Sophus/sophus/so3.hpp"
#include <cmath>
#include <limits>

namespace geometry {

///
/// \brief Convert calibration::CoordinateTransformation into Sophus::SE3
///
template <typename T> void getSophusSE3(Sophus::SE3<T>& se3, const calibration::CoordinateTransformation& pose) {
    const T rx = pose.rodriguesrotationx();
    const T ry = pose.rodriguesrotationy();
    const T rz = pose.rodriguesrotationz();
    const T tx = pose.translationx();
    const T ty = pose.translationy();
    const T tz = pose.translationz();

    const T angle = std::sqrt(rx * rx + ry * ry + rz * rz);
    Sophus::SO3<T> so3;
    if (std::abs(angle) < std::numeric_limits<T>::epsilon()) {
        so3 = Sophus::SO3<T>(Eigen::Quaternion<T>(Eigen::AngleAxis<T>(0, Eigen::Matrix<T, 3, 1>(T(1), T(0), T(0)))));
    } else {
        so3 = Sophus::SO3<T>(Eigen::Quaternion<T>(Eigen::AngleAxis<T>(angle, Eigen::Matrix<T, 3, 1>(rx / angle, ry / angle, rz / angle))));
    }
    se3 = Sophus::SE3<T>(so3, typename Sophus::SE3<T>::Point(tx, ty, tz));
}

///
/// \brief Convert calibration::CoordinateTransformation into Sophus::SE3
///
template <typename T> Sophus::SE3<T> getSophusSE3(const calibration::CoordinateTransformation& pose) {
    Sophus::SE3<T> se3;
    getSophusSE3<T>(se3, pose);
    return se3;
}
}
