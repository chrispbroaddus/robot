#pragma once

#include "Eigen/Dense"

namespace calibration {
template <typename T> class LinearCameraModel {
public:
    /// Instantiate a Linear Camera Model with the specified intrinsic parameters
    /// \param K Calibration matri
    LinearCameraModel(const Eigen::Matrix<T, 3, 3>& K)
        : m_K(K) {}

    /// Project a ray into a image space.
    /// \param ray Source ray
    /// \param K Calibration matrix
    /// \return 2D image point
    Eigen::Matrix<T, 2, 1> project(const Eigen::Matrix<T, 3, 1>& ray) { return (m_K * ray.hnormalized().homogeneous()).hnormalized(); }

    /// Unproject a image point into a ray.
    /// \param m Source 2D image point
    /// \param K Calibration matrix
    /// \return Unprojected ray
    Eigen::Matrix<T, 3, 1> unproject(const Eigen::Matrix<T, 2, 1>& m) { return m_K.inverse() * m.homogeneous(); }

private:
    const Eigen::Matrix<T, 3, 3>& m_K;
};
}
