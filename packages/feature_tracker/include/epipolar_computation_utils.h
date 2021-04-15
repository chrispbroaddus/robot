#pragma once

#include "Eigen/Eigen"
#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"

namespace feature_tracker {

/// Computes the essential matrix given the rotation matrix and translation vector to convert a point from the right camera's
/// coordinate system to the left camera's coordinate system
/// \tparam T data-type of the matrix structures
/// \param rotationMatrix
/// \param translationVector
/// \return the essential matrix
template <typename T>
inline Eigen::Matrix<T, 3, 3> computeEssentialMatrix(
    const Eigen::Matrix<T, 3, 3>& rotationMatrix, const Eigen::Matrix<T, 3, 1>& translationVector) {

    Eigen::Matrix<T, 3, 3> crossProductMatrix;
    crossProductMatrix << 0, -translationVector(2, 0), translationVector(1, 0), translationVector(2, 0), 0, -translationVector(0, 0),
        -translationVector(1, 0), translationVector(0, 0), 0;

    return crossProductMatrix * rotationMatrix;
}

/// Computes the epipolar line in the right image for a point in the calibrated space(Undistorted and unit focal length) of left image
/// \tparam T data-type of the matrix structures
/// \param essentialMatrix Essential Matrix computed using the extrinsic transformation from right to left camera
/// \param leftCalibratedPoint A point in the calibrated space(Undistorted and unit focal length) of left image
/// \return the epipolar line for the right image
template <typename T>
inline Eigen::Matrix<T, 3, 1> computeRightEpipolarLine(
    const Eigen::Matrix<T, 3, 3>& essentialMatrix, const Eigen::Matrix<T, 3, 1>& leftCalibratedPoint) {
    return (leftCalibratedPoint.transpose() * essentialMatrix).transpose();
}

/// Computes the epipolar line in the left image for a point in the calibrated space(Undistorted and unit focal length) in right image
/// \tparam T data-type of the matrix structures
/// \param essentialMatrix Essential Matrix computed using the extrinsic transformation from right to left camera
/// \param rightCalibratedPoint
/// \return the epipolar line for the left image
template <typename T>
inline Eigen::Matrix<T, 3, 1> computeLeftEpipolarLine(
    const Eigen::Matrix<T, 3, 3>& essentialMatrix, const Eigen::Matrix<T, 3, 1>& rightCalibratedPoint) {
    return (essentialMatrix * rightCalibratedPoint);
}

/// Computes the shortest distance from a point to a line
/// \tparam T data-type of the matrix structures
/// \param point homogenous coordinates of a 2D point
/// \param line vector of coefficients of the line
/// \return returns the shortest(perpendicular) distance from the point to the line
template <typename T> inline T shortestDistPointToLine(const Eigen::Matrix<T, 3, 1>& point, const Eigen::Matrix<T, 3, 1>& line) {

    if (std::abs(1.0f - point(2, 0)) > std::numeric_limits<T>::epsilon()) {
        throw std::runtime_error("shortestDistPointToLine: point should have z value of 1");
    }
    return std::abs(line.dot(point)) / std::sqrt(line(0, 0) * line(0, 0) + line(1, 0) * line(1, 0));
}
}