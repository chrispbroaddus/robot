
#pragma once

#include "packages/imu_propagator/include/imu_propagator_utils.h"

#include "Eigen/Dense"
#include "Eigen/Geometry"
#include <iostream>

namespace imu_propagator {
namespace details {

    template <typename SCALAR> using F_type = Eigen::Matrix<SCALAR, 15, 15>;
    template <typename SCALAR> using G_type = Eigen::Matrix<SCALAR, 15, 12>;
    template <typename SCALAR> using Q_type = Eigen::Matrix<SCALAR, 12, 12>;
    template <typename SCALAR> using state_type = Eigen::Matrix<SCALAR, 15, 1>;
    template <typename SCALAR> using residual_type = Eigen::Matrix<SCALAR, 15, 1>;
    template <typename SCALAR> using gyro_type = Eigen::Matrix<SCALAR, 3, 1>;
    template <typename SCALAR> using accel_type = Eigen::Matrix<SCALAR, 3, 1>;
    template <typename SCALAR> using gravity_type = Eigen::Matrix<SCALAR, 3, 1>;
    template <typename SCALAR> using matrix3_type = Eigen::Matrix<SCALAR, 3, 3>;
    template <typename SCALAR> using vector3_type = Eigen::Matrix<SCALAR, 3, 1>;

    /// These are the IMU system equations in:
    /// A Multi-State Constraint Kalman Filter for Vision-aided Inertial Navigation, Anastasios I. Mourikis and Stergios I. Roumeliotis
    template <typename SCALAR> class PropagationEquations {
    public:
        /// Compute the continuous time state space matrix F.
        /// \param q quaternion for evaluating F
        /// \param gyro gyroscope sample
        /// \param accel accelerometer sample
        /// \return state space matrix
        static F_type<SCALAR> F(const Eigen::Quaternion<SCALAR>& q, const gyro_type<SCALAR>& gyro, const accel_type<SCALAR>& accel) {
            F_type<SCALAR> F = F_type<SCALAR>::Zero();

            // F00
            const matrix3_type<SCALAR> F00 = -utils::skewSymmetricMatrix(gyro);

            // F01
            const matrix3_type<SCALAR> F01 = -matrix3_type<SCALAR>::Identity();

            // F20
            const matrix3_type<SCALAR> F20 = -q.toRotationMatrix() * utils::skewSymmetricMatrix(accel);

            // F23
            const matrix3_type<SCALAR> F23 = -q.toRotationMatrix();

            // F42
            const matrix3_type<SCALAR> F42 = matrix3_type<SCALAR>::Identity();

            F.block(0, 0, 3, 3) = F00;
            F.block(0, 3, 3, 3) = F01;
            F.block(6, 0, 3, 3) = F20;
            F.block(6, 9, 3, 3) = F23;
            F.block(12, 6, 3, 3) = F42;

            return F;
        }

        /// Compute the matrix G such that G*Q*G' is the oriented process noise.
        /// \param q quaternion for evaluating G
        /// \return matrix G
        static G_type<SCALAR> G(const Eigen::Quaternion<double>& q) {
            G_type<SCALAR> G = G_type<SCALAR>::Zero();

            // G00
            const matrix3_type<SCALAR> G00 = -matrix3_type<SCALAR>::Identity();

            // G11
            const matrix3_type<SCALAR> G11 = matrix3_type<SCALAR>::Identity();

            // G22
            const matrix3_type<SCALAR> G22 = -q.toRotationMatrix();

            // G33
            const matrix3_type<SCALAR> G33 = matrix3_type<SCALAR>::Identity();

            G.block(0, 0, 3, 3) = G00;
            G.block(3, 3, 3, 3) = G11;
            G.block(6, 6, 3, 3) = G22;
            G.block(9, 9, 3, 3) = G33;

            return G;
        }

        /// Compute the residual vector given two state vectors. The special case here is the orientation error which
        /// is represented as radians.
        /// \param left
        /// \param right
        /// \return residual vector
        static residual_type<SCALAR> residual(const state_type<SCALAR>& left, const state_type<SCALAR>& right) {
            residual_type<SCALAR> r = left - right;

            r.block(0, 0, 3, 1) *= 2;

            return r;
        }
    };
}
}
