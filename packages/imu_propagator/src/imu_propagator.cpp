
#include "packages/imu_propagator/include/imu_propagator.h"
#include "packages/imu_propagator/include/runge_kutta_integrator.h"

namespace imu_propagator {

template <typename TIMESTAMP, typename SCALAR>
details::state_type<SCALAR> ImuPropagator<TIMESTAMP, SCALAR>::propagate(const TIMESTAMP timestamp, const details::state_type<SCALAR>& state,
    const typename covariance_ode_type::y_type& covariance, const imu_database_type& imuDatabase, const TIMESTAMP nextTimestamp) {

    // Get all the IMU samples in the range [timestamp, nextTimestamp]
    const auto imuSamples = imuDatabase.inRange(std::make_tuple(timestamp, nextTimestamp));

    // Construct a noise covariance matrix
    details::Q_type<SCALAR> Q = details::Q_type<SCALAR>::Zero();
    Q.diagonal().segment(0, 3) = m_gyroWhiteNoise.cwiseProduct(m_gyroWhiteNoise);
    Q.diagonal().segment(3, 3) = m_gyroBiasNoise.cwiseProduct(m_gyroBiasNoise);
    Q.diagonal().segment(6, 3) = m_accelWhiteNoise.cwiseProduct(m_accelWhiteNoise);
    Q.diagonal().segment(9, 3) = m_accelBiasNoise.cwiseProduct(m_accelBiasNoise);

    details::state_type<SCALAR> initialState = state;
    details::state_type<SCALAR> nextState = details::state_type<SCALAR>::Zero();
    typename jacobian_ode_type::y_type initialPhi = jacobian_ode_type::y_type::Identity();
    typename covariance_ode_type::y_type initialCov = covariance;
    m_nextPhi = jacobian_ode_type::y_type::Zero();
    m_nextCov = covariance_ode_type::y_type::Zero();

    for (auto it = imuSamples.begin(); std::next(it) != imuSamples.end(); it++) {
        const auto gyroBias = nextState.block(3, 0, 3, 1);
        const auto accelBias = nextState.block(9, 0, 3, 1);

        const auto step = std::next(it)->timestamp() - it->timestamp();

        //
        // Integrate the state
        //
        state_ode_type stateOde(initialState, it->timestamp(), m_gravity, imuDatabase);
        RungeKutta4thOrderIntegrator<TIMESTAMP, StateODE<SCALAR> > stateIntegrator;
        nextState = stateIntegrator.integrate(stateOde, step);

        //
        // Integrate the Jacobian
        //
        const details::vector3_type<SCALAR> initialQuatVec = initialState.block(0, 0, 3, 1);
        const details::vector3_type<SCALAR> nextQuatVec = nextState.block(0, 0, 3, 1);
        jacobian_ode_type jacobianOde(initialPhi, it->timestamp(), gyroBias, accelBias, utils::imaginaryToQuaternion(initialQuatVec),
            utils::imaginaryToQuaternion(nextQuatVec), imuDatabase);
        RungeKutta4thOrderIntegrator<TIMESTAMP, jacobian_ode_type> jacobianIntegrator;
        m_nextPhi = jacobianIntegrator.integrate(jacobianOde, step);

        //
        // Integrate the Covariance
        //
        covariance_ode_type covarianceOde(initialCov, it->timestamp(), gyroBias, accelBias, Q, utils::imaginaryToQuaternion(initialQuatVec),
            utils::imaginaryToQuaternion(nextQuatVec), imuDatabase);
        RungeKutta4thOrderIntegrator<SCALAR, covariance_ode_type> covarianceIntegrator;
        m_nextCov = covarianceIntegrator.integrate(covarianceOde, step);

        initialState = nextState;
        initialPhi = m_nextPhi;
        initialCov = m_nextCov;
    }

    return nextState;
}
}

template class imu_propagator::ImuPropagator<double, double>;
