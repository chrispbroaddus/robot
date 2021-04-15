
#pragma once

#include "packages/imu_propagator/include/imu_database.h"
#include "packages/imu_propagator/include/imu_propagator_details.h"
#include "packages/imu_propagator/include/imu_propagator_utils.h"

#include "Eigen/Dense"

namespace imu_propagator {

/// Ordinary Differential Equation (ODE) for the IMU state (quaternion, gyroBias, velocity, accelBias, position).
///
///     quaternion: rotation from IMU to world
///     gyroBias: bias of the gyroscope
///     velocity: velocity of the IMU in m/s
///     accelBias: bias of the accelerometer
///     position: position of the IMU in the world
///
/// \tparam SCALAR precision
template <typename SCALAR> class StateODE {
public:
    static constexpr size_t y_rows = 15;
    static constexpr size_t y_cols = 1;
    typedef Eigen::Matrix<SCALAR, y_rows, y_cols> y_type;
    typedef Eigen::Matrix<SCALAR, y_rows, y_cols> ydot_type;
    typedef ImuDatabase<double, SCALAR> imu_database_type;
    typedef LinearImuInterpolator<double, SCALAR> imu_interpolator_type;

    StateODE(const y_type& y0, const SCALAR t0, const details::gravity_type<SCALAR>& gravity, const imu_database_type& imuDatabase)
        : m_y0(y0)
        , m_t0(t0)
        , m_gravity(gravity)
        , m_imuDatabase(imuDatabase) {}
    ~StateODE() = default;

    /// Evaluate the ODE at time t and state y.
    /// \param t time t
    /// \param y state at time t
    /// \return derivative at time t
    ydot_type evaluate(const SCALAR& t, const y_type& y, const SCALAR /*step*/) const {
        ydot_type ydot;
        ydot.setZero();

        const Eigen::Matrix<SCALAR, 3, 1> vecQ = y.block(0, 0, 3, 1);
        const Eigen::Matrix<SCALAR, 3, 1> gyroBias = y.block(3, 0, 3, 1);
        const Eigen::Matrix<SCALAR, 3, 1> velocity = y.block(6, 0, 3, 1);
        const Eigen::Matrix<SCALAR, 3, 1> accelBias = y.block(9, 0, 3, 1);

        const Eigen::Quaternion<SCALAR> q = utils::imaginaryToQuaternion(vecQ);

        imu_interpolator_type interpolator(m_imuDatabase);
        typename imu_interpolator_type::imu_sample_type imuSample = interpolator.getImuSampleAtTime(t);

        const details::gyro_type<SCALAR> gyroCorrected = imuSample.gyro() - gyroBias;
        const details::accel_type<SCALAR> accelCorrected = imuSample.accel() - accelBias;

        const Eigen::Matrix<SCALAR, 4, 1> qp = 0.5 * utils::omega(gyroCorrected) * q.coeffs();

        ydot.block(0, 0, 3, 1) = qp.block(0, 0, 3, 1);
        ydot.block(6, 0, 3, 1) = q.toRotationMatrix() * accelCorrected + m_gravity;
        ydot.block(12, 0, 3, 1) = velocity;

        return ydot;
    }

    const y_type& initialValue() const { return m_y0; }
    SCALAR t0() const { return m_t0; }

private:
    const y_type m_y0;
    const SCALAR m_t0;
    const details::gravity_type<SCALAR> m_gravity;
    const imu_database_type& m_imuDatabase;
};

/// Jacobian Ordinary Differential Equation (ODE) for recovering the Jacobian of J = dx(t+dt)/dx(t) such that
/// x(t+dt) = J*x(t).
/// \tparam SCALAR precision
template <typename SCALAR> class JacobianODE {
public:
    static constexpr size_t y_rows = 15;
    static constexpr size_t y_cols = 15;
    typedef Eigen::Matrix<SCALAR, y_rows, y_cols> y_type;
    typedef Eigen::Matrix<SCALAR, y_rows, y_cols> ydot_type;
    typedef ImuDatabase<double, SCALAR> imu_database_type;
    typedef LinearImuInterpolator<double, SCALAR> imu_interpolator_type;

    JacobianODE(const y_type& y0, const SCALAR t0, const details::gyro_type<SCALAR>& gyroBias, const details::accel_type<SCALAR>& accelBias,
        const Eigen::Quaternion<SCALAR>& initialQuat, const Eigen::Quaternion<SCALAR>& nextQuat, const imu_database_type& imuDatabase)
        : m_y0(y0)
        , m_t0(t0)
        , m_gyroBias(gyroBias)
        , m_accelBias(accelBias)
        , m_initialQuat(initialQuat)
        , m_nextQuat(nextQuat)
        , m_imuDatabase(imuDatabase) {}

    ydot_type evaluate(const SCALAR& t, const y_type& y, const SCALAR step) const {
        // Parameter between [0,1] for SLERP
        const auto dt = (t - m_t0) / step;
        const Eigen::Quaternion<SCALAR> q = m_initialQuat.slerp(dt, m_nextQuat);

        imu_interpolator_type interpolator(m_imuDatabase);
        typename imu_interpolator_type::imu_sample_type imuSample = interpolator.getImuSampleAtTime(t);

        const details::gyro_type<SCALAR> gyroCorrected = imuSample.gyro() - m_gyroBias;
        const details::accel_type<SCALAR> accelCorrected = imuSample.accel() - m_accelBias;

        const auto F = details::PropagationEquations<SCALAR>::F(q, gyroCorrected, accelCorrected);

        return F * y;
    }

    const y_type& initialValue() const { return m_y0; }
    SCALAR t0() const { return m_t0; }

private:
    const y_type m_y0;
    const SCALAR m_t0;
    const details::gyro_type<SCALAR> m_gyroBias;
    const details::accel_type<SCALAR> m_accelBias;
    const Eigen::Quaternion<SCALAR> m_initialQuat;
    const Eigen::Quaternion<SCALAR> m_nextQuat;
    const imu_database_type& m_imuDatabase;
};

/// Covariance Ordinary Differential Equation (ODE) for recovering the propagated covariance from t to t+dt.
/// \tparam SCALAR precision
template <typename SCALAR> class CovarianceODE {
public:
    static constexpr size_t y_rows = 15;
    static constexpr size_t y_cols = 15;
    typedef Eigen::Matrix<SCALAR, y_rows, y_cols> y_type;
    typedef Eigen::Matrix<SCALAR, y_rows, y_cols> ydot_type;
    typedef ImuDatabase<double, SCALAR> imu_database_type;
    typedef LinearImuInterpolator<double, SCALAR> imu_interpolator_type;

    CovarianceODE(const y_type& y0, const SCALAR t0, const details::gyro_type<SCALAR>& gyroBias,
        const details::accel_type<SCALAR>& accelBias, const details::Q_type<SCALAR>& Q, const Eigen::Quaternion<SCALAR>& initialQuat,
        const Eigen::Quaternion<SCALAR>& nextQuat, const imu_database_type& imuDatabase)
        : m_y0(y0)
        , m_t0(t0)
        , m_gyroBias(gyroBias)
        , m_accelBias(accelBias)
        , m_Q(Q)
        , m_initialQuat(initialQuat)
        , m_nextQuat(nextQuat)
        , m_imuDatabase(imuDatabase) {}

    ydot_type evaluate(const SCALAR& t, const y_type& y, const SCALAR step) const {
        // Parameter between [0,1] for SLERP
        const auto dt = (t - m_t0) / step;
        const Eigen::Quaternion<SCALAR> q = m_initialQuat.slerp(dt, m_nextQuat);

        imu_interpolator_type interpolator(m_imuDatabase);
        typename imu_interpolator_type::imu_sample_type imuSample = interpolator.getImuSampleAtTime(t);

        const details::gyro_type<SCALAR> gyroCorrected = imuSample.gyro() - m_gyroBias;
        const details::accel_type<SCALAR> accelCorrected = imuSample.accel() - m_accelBias;

        const auto F = details::PropagationEquations<SCALAR>::F(q, gyroCorrected, accelCorrected);
        const auto G = details::PropagationEquations<SCALAR>::G(q);

        return (F * y + y * F.transpose()) + (G * m_Q * G.transpose());
    }

    const y_type& initialValue() const { return m_y0; }
    SCALAR t0() const { return m_t0; }

private:
    const y_type m_y0;
    const SCALAR m_t0;
    const details::gyro_type<SCALAR> m_gyroBias;
    const details::accel_type<SCALAR> m_accelBias;
    const details::Q_type<SCALAR> m_Q;
    const Eigen::Quaternion<SCALAR> m_initialQuat;
    const Eigen::Quaternion<SCALAR> m_nextQuat;
    const imu_database_type& m_imuDatabase;
};
}
