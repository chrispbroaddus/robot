
#pragma once

#include "packages/imu_propagator/include/imu_database.h"
#include "packages/imu_propagator/include/imu_propagator_ode.h"

namespace imu_propagator {

/// This is the highest level of the IMU propagator which implements the integrators for gyroscope and accelerometer to recover
/// the state, jacobian and covariance.
template <typename TIMESTAMP, typename SCALAR> class ImuPropagator {
public:
    typedef ImuDatabase<TIMESTAMP, SCALAR> imu_database_type;
    typedef StateODE<SCALAR> state_ode_type;
    typedef JacobianODE<SCALAR> jacobian_ode_type;
    typedef CovarianceODE<SCALAR> covariance_ode_type;
    typedef Eigen::Matrix<SCALAR, 3, 1> noise_type;

    ImuPropagator(const noise_type& gyroWhiteNoise, const noise_type& gyroBiasNoise, const noise_type& accelWhiteNoise,
        const noise_type& accelBiasNoise, const details::gravity_type<SCALAR>& gravity)
        : m_gyroWhiteNoise(gyroWhiteNoise)
        , m_gyroBiasNoise(gyroBiasNoise)
        , m_accelWhiteNoise(accelWhiteNoise)
        , m_accelBiasNoise(accelBiasNoise)
        , m_gravity(gravity) {}
    ~ImuPropagator() = default;

    /// Propagate the state and covariance.
    /// \param timestamp the timestamp of the state
    /// \param state the state vector
    /// \param covariance the covariance of the state vector
    /// \param imuDatabase the IMU database which has sufficient IMU data from [timestamp, nextTimestamp]
    /// \param nextTimestamp the timestamp to propagate the state to
    /// \return the propagated state
    details::state_type<SCALAR> propagate(const TIMESTAMP timestamp, const details::state_type<SCALAR>& state,
        const typename covariance_ode_type::y_type& covariance, const imu_database_type& imuDatabase, const TIMESTAMP nextTimestamp);

    /// \return the Jacobian J = dx(t+dt)/dx(t) such that x(t+dt) = J*x(t)
    const typename jacobian_ode_type::y_type& jacobian() const { return m_nextPhi; }

    /// \return the covariance of the state after propagation
    const typename covariance_ode_type::y_type& covariance() const { return m_nextCov; }

private:
    const noise_type m_gyroWhiteNoise;
    const noise_type m_gyroBiasNoise;
    const noise_type m_accelWhiteNoise;
    const noise_type m_accelBiasNoise;

    typename jacobian_ode_type::y_type m_nextPhi;
    typename covariance_ode_type::y_type m_nextCov;

    const details::gravity_type<SCALAR> m_gravity;
};
}
