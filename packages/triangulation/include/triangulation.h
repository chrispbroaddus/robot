#pragma once

#include "Eigen/Dense"
#include <limits>

namespace triangulation {
/// Invert the rigid transform (R,t).
template <typename T>
inline void invertRigidTransform(
    Eigen::Matrix<T, 3, 3>& Rinv, Eigen::Matrix<T, 3, 1>& tinv, const Eigen::Matrix<T, 3, 3>& R, const Eigen::Matrix<T, 3, 1>& t) {
    Rinv = R.transpose();
    tinv = -Rinv * t;
}

/// Compound rigid transforms (R0,t0) and (R1,t1).
/// y0 = R0*x0+t0
/// y1 = R1*x1+t1, let x1 = y0
/// y2 = R1*y0+t1 = R1*(R0*x0+t0)+t1 = (R1*R0)*x0+(R1*t0+t1) = R2*x0+t2
template <typename T>
inline void compoundRigidTransform(Eigen::Matrix<T, 3, 3>& R2, Eigen::Matrix<T, 3, 1>& t2, const Eigen::Matrix<T, 3, 3>& R0,
    const Eigen::Matrix<T, 3, 1>& t0, const Eigen::Matrix<T, 3, 3>& R1, const Eigen::Matrix<T, 3, 1>& t1) {
    R2 = R1 * R0;
    t2 = R1 * t0 + t1;
}

/// Equation 16 in "Exact Two–Image Structure from Motion, John Oliensis"
template <typename T> inline T computeG(const Eigen::Matrix<T, 3, 1>& p0, const Eigen::Matrix<T, 3, 1>& p1) {
    return std::sqrt(std::pow(p0(0) * p0(0) - p1(0) * p1(0) - p1(1) * p1(1), 2) + 4 * p0(0) * p0(0) * p1(0) * p1(0));
}

/// Equation 34 in "Exact Two–Image Structure from Motion, John Oliensis"
template <typename T> inline T computeDprime(const Eigen::Matrix<T, 3, 1>& p0, const Eigen::Matrix<T, 3, 1>& p1, const T& G) {
    return 2 * p1(0) * p1(2) * p0(0) + p0(2) * (p0(0) * p0(0) - p1(0) * p1(0) - p1(1) * p1(1) - G);
}

/// Equation 35 in "Exact Two–Image Structure from Motion, John Oliensis"
template <typename T> inline T computeL0(const Eigen::Matrix<T, 3, 1>& p0, const Eigen::Matrix<T, 3, 1>& p1, const T& G, const T& Dprime) {
    if (std::abs(p1(1)) <= std::numeric_limits<T>::epsilon()) {
        return -p1(0) / (p0(0) * p1(2) - p0(2) * p1(0));
    } else {
        return (p0(0) * p0(0) - p1(0) * p1(0) - p1(1) * p1(1) - G) / Dprime;
    }
}

/// Implements the direction triangulation algorithm in "Exact Two–Image Structure from Motion, John Oliensis".
/// \param M Triangulated 3d point
/// \param m0 Calibrated pixel location in image 0
/// \param R1 Rotation from from camera 1 to 0
/// \param t1 Position of camera 1 in camera 0
/// \param m1 Calibration pixel location in image 1
/// \return true if success, false otherwise
template <typename T>
inline bool triangulateDirectional(Eigen::Matrix<T, 3, 1>& M, const Eigen::Matrix<T, 2, 1>& m0, const Eigen::Matrix<T, 3, 3>& R1,
    const Eigen::Matrix<T, 3, 1>& t1, const Eigen::Matrix<T, 2, 1>& m1) {

    const Eigen::Matrix<T, 3, 1> p0 = m0.homogeneous().normalized();
    const Eigen::Matrix<T, 3, 1> p1 = m1.homogeneous().normalized();
    const Eigen::Matrix<T, 3, 1> T1normalized = t1.normalized();

    const Eigen::Matrix<T, 3, 1> p1u = R1 * p1;

    const Eigen::Matrix<T, 3, 1> Qcol1 = (p0 - T1normalized * (T1normalized.dot(p0))).normalized();
    const Eigen::Matrix<T, 3, 1> Qcol2 = T1normalized.cross(p0).normalized();
    const Eigen::Matrix<T, 3, 1> Qcol3 = T1normalized;

    Eigen::Matrix<T, 3, 3> Q;
    Q.template block<1, 3>(0, 0) = Qcol1.transpose();
    Q.template block<1, 3>(1, 0) = Qcol2.transpose();
    Q.template block<1, 3>(2, 0) = Qcol3.transpose();

    const Eigen::Matrix<T, 3, 1> p0p = Q * p0;
    const Eigen::Matrix<T, 3, 1> p1up = Q * p1u;

    const T G = computeG(p0p, p1up);
    const T Dp = computeDprime(p0p, p1up, G);
    if (std::abs(Dp) <= std::numeric_limits<T>::epsilon()) {
        return false;
    }
    const T L0 = computeL0(p0p, p1up, G, Dp);
    const T s = p0p(0) / (2 * G);

    Eigen::Matrix<T, 3, 1> Mp;
    Mp(0) = L0 * (s * (p0p(0) * p0p(0) + p1up(0) * p1up(0) - p1up(1) * p1up(1) + G));
    Mp(1) = L0 * (s * (2 * p1up(0) * p1up(1)));
    Mp(2) = L0 * p0p(2);

    M = (Q.transpose() * Mp) * t1.norm();

    return true;
}

/// Implements the direction triangulation algorithm in "Exact Two–Image Structure from Motion, John Oliensis".
/// \param M Triangulated 3d point
/// \param R0 Rotation from from camera 1 to world
/// \param t0 Position of camera 0 in world
/// \param m0 Calibrated pixel location in image 0
/// \param R1 Rotation from from camera 1 to world
/// \param t1 Position of camera 1 in world
/// \param m1 Calibrated pixel location in image 1
/// \return true if success, false otherwise
template <typename T>
inline bool triangulateDirectional(Eigen::Matrix<T, 3, 1>& M, const Eigen::Matrix<T, 3, 3>& R0, const Eigen::Matrix<T, 3, 1>& t0,
    const Eigen::Matrix<T, 2, 1>& m0, const Eigen::Matrix<T, 3, 3>& R1, const Eigen::Matrix<T, 3, 1>& t1,
    const Eigen::Matrix<T, 2, 1>& m1) {

    Eigen::Matrix<T, 3, 3> R0inv;
    Eigen::Matrix<T, 3, 1> t0inv;
    invertRigidTransform(R0inv, t0inv, R0, t0);

    Eigen::Matrix<T, 3, 3> R;
    Eigen::Matrix<T, 3, 1> t;
    compoundRigidTransform(R, t, R1, t1, R0inv, t0inv);

    Eigen::Matrix<T, 3, 1> Mp;
    if (!triangulateDirectional(Mp, m0, R, t, m1)) {
        return false;
    }

    M = R0 * Mp + t0;

    return true;
}
}