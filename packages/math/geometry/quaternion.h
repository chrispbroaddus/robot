#pragma once

#include "Eigen/Eigen"
#include <cmath>
#include <limits>

namespace geometry {

///
/// \brief Convert rodrigues rotation angles to quaternion
/// \details https://zippy.quip.com/DmuHAbce5MVP
///
template <typename T> inline void convertRodriguesToQuaternion(T& qw, T& qx, T& qy, T& qz, const T rx, const T ry, const T rz) {
    T theta = std::sqrt(rx * rx + ry * ry + rz * rz);
    if (std::fabs(theta) < std::numeric_limits<T>::epsilon()) {
        qw = 1;
        qx = 0;
        qy = 0;
        qz = 0;
    } else {
        qw = std::cos(theta / 2);
        T s = std::sin(theta / 2);
        qx = rx * s / theta;
        qy = ry * s / theta;
        qz = rz * s / theta;
    }
}

///
/// \brief Convert rodrigues rotation angles to quaternion
///
template <typename T> inline void convertRodriguesToQuaternion(Eigen::Quaternion<T>& q, const T rx, const T ry, const T rz) {
    convertRodriguesToQuaternion(q.w(), q.x(), q.y(), q.z(), rx, ry, rz);
}

///
/// \brief Convert rodrigues rotation angles to quaternion
///
template <typename T> inline Eigen::Quaternion<T> convertRodriguesToQuaternion(const T rx, const T ry, const T rz) {
    Eigen::Quaternion<T> q;
    convertRodriguesToQuaternion(q, rx, ry, rz);
    return q;
}

///
/// \brief Convert quaternion to rodrigues rotation angles
/// \details https://zippy.quip.com/DmuHAbce5MVP
///
template <typename T> inline void convertQuaternionToRodrigues(T& rx, T& ry, T& rz, const T qw, const T qx, const T qy, const T qz) {
    T thetaOverTwo = std::acos(qw);
    if (std::fabs(thetaOverTwo) < std::numeric_limits<T>::epsilon()) {
        rx = 0;
        ry = 0;
        rz = 0;
    } else {
        T s = std::sin(thetaOverTwo);
        rx = qx * thetaOverTwo * 2 / s;
        ry = qy * thetaOverTwo * 2 / s;
        rz = qz * thetaOverTwo * 2 / s;
    }
}

///
/// \brief Convert quaternion to rodrigues rotation angles
///
template <typename T> inline void convertQuaternionToRodrigues(Eigen::Matrix<T, 3, 1>& r, const T qw, const T qx, const T qy, const T qz) {
    convertQuaternionToRodrigues(r(0, 0), r(1, 0), r(2, 0), qw, qx, qy, qz);
}

///
/// \brief Convert quaternion to rodrigues rotation angles
///
template <typename T> inline Eigen::Matrix<T, 3, 1> convertQuaternionToRodrigues(const T qw, const T qx, const T qy, const T qz) {
    Eigen::Matrix<T, 3, 1> r;
    convertQuaternionToRodrigues(r, qw, qx, qy, qz);
    return r;
}
}
