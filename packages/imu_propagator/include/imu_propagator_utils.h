
#pragma once

#include "Eigen/Dense"
#include <stdexcept>

namespace imu_propagator {
namespace utils {

    /// Compute the skew-symmetric matrix.
    /// \param A output skew-symmetric matrix
    /// \param x source vector
    template <typename SCALAR> inline const Eigen::Matrix<SCALAR, 3, 3> skewSymmetricMatrix(const Eigen::Matrix<SCALAR, 3, 1>& x) {
        Eigen::Matrix<SCALAR, 3, 3> A;
        A(0, 0) = 0;
        A(0, 1) = -x(2);
        A(0, 2) = x(1);
        A(1, 0) = x(2);
        A(1, 1) = 0;
        A(1, 2) = -x(0);
        A(2, 0) = -x(1);
        A(2, 1) = x(0);
        A(2, 2) = 0;
        return A;
    }

    /// Construct the Omega matrix in:
    /// Equation 6 in A Multi-State Constraint Kalman Filter for Vision-aided Inertial Navigation, Anastasios I. Mourikis and Stergios I.
    /// Roumeliotis
    template <typename SCALAR> inline const Eigen::Matrix<SCALAR, 4, 4> omega(const Eigen::Matrix<SCALAR, 3, 1>& x) {
        Eigen::Matrix<SCALAR, 4, 4> A;
        A.setZero();
        A.block(0, 0, 3, 3) = -skewSymmetricMatrix(x);
        A.block(0, 3, 3, 1) = x;
        A.block(3, 0, 1, 3) = -x.transpose();
        return A;
    }

    /// Convert the imaginary components of the quaternion to a full quaternion.
    template <typename SCALAR> inline Eigen::Quaternion<SCALAR> imaginaryToQuaternion(const Eigen::Matrix<SCALAR, 3, 1>& imaginary) {
        const SCALAR x = 1 - imaginary.dot(imaginary);
        if (x < 0) {
            throw std::runtime_error("cannot get a quaternion from the imaginary components");
        }
        return Eigen::Quaternion<SCALAR>(std::sqrt(x), imaginary(0, 0), imaginary(1, 0), imaginary(2, 0));
    }
}
}
