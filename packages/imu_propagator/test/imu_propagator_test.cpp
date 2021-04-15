
#include "packages/imu_propagator/include/imu_propagator.h"
#include "packages/imu_propagator/include/imu_propagator_details.h"
#include "packages/imu_propagator/include/imu_propagator_ode.h"
#include "packages/imu_propagator/include/imu_propagator_utils.h"
#include "packages/imu_propagator/include/runge_kutta_integrator.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace imu_propagator;
using namespace imu_propagator::details;
using namespace imu_propagator::utils;

TEST(skewSymmetricMatrix, regression) {
    const Eigen::Vector3d x(1, 2, 3);

    const Eigen::Matrix3d A = skewSymmetricMatrix(x);

    EXPECT_DOUBLE_EQ(0, A(0, 0));
    EXPECT_DOUBLE_EQ(-x(2), A(0, 1));
    EXPECT_DOUBLE_EQ(x(1), A(0, 2));
    EXPECT_DOUBLE_EQ(x(2), A(1, 0));
    EXPECT_DOUBLE_EQ(0, A(1, 1));
    EXPECT_DOUBLE_EQ(-x(0), A(1, 2));
    EXPECT_DOUBLE_EQ(-x(1), A(2, 0));
    EXPECT_DOUBLE_EQ(x(0), A(2, 1));
    EXPECT_DOUBLE_EQ(0, A(2, 2));
}

TEST(StateODE, RungeKutta4thOrderIntegratorGyro) {
    typedef StateODE<double> ode_type;
    typedef ImuDatabase<double, double> imu_database_type;

    const double oneDegreeInRadians = 1 * M_PI / 180;

    constexpr double t0 = 99;
    const details::gravity_type<double> gravity = { 0, 0, 0 };
    const ode_type::y_type y0 = ode_type::y_type::Zero();

    for (size_t i = 0; i < 3; i++) {
        imu_database_type imuDatabase(10);

        imu_database_type::imu_sample_type::gyro_type gyro1(0, 0, 0);
        imu_database_type::imu_sample_type::accel_type accel1(0, 0, 0);
        gyro1(i, 0) = oneDegreeInRadians;

        imu_database_type::imu_sample_type::gyro_type gyro2(0, 0, 0);
        imu_database_type::imu_sample_type::accel_type accel2(0, 0, 0);
        gyro2(i, 0) = oneDegreeInRadians;

        const imu_database_type::imu_sample_type imu1(99, gyro1, accel1);
        const imu_database_type::imu_sample_type imu2(101, gyro2, accel2);

        imuDatabase.addImuSample(imu1);
        imuDatabase.addImuSample(imu2);

        ode_type ode(y0, t0, gravity, imuDatabase);

        RungeKutta4thOrderIntegrator<double, StateODE<double> > integrator;
        const ode_type::y_type y = integrator.integrate(ode, 1);

        EXPECT_NEAR(oneDegreeInRadians, 2 * y(i, 0), 1e-4);
    }
}

TEST(StateODE, RungeKutta4thOrderIntegratorAccel) {
    typedef StateODE<double> ode_type;
    typedef ImuDatabase<double, double> imu_database_type;

    constexpr double t0 = 99;
    const details::gravity_type<double> gravity = { 0, 0, 0 };
    const ode_type::y_type y0 = ode_type::y_type::Zero();

    for (size_t i = 0; i < 3; i++) {
        imu_database_type imuDatabase(10);

        imu_database_type::imu_sample_type::gyro_type gyro1(0, 0, 0);
        imu_database_type::imu_sample_type::accel_type accel1(0, 0, 0);
        accel1(i, 0) = 1;

        imu_database_type::imu_sample_type::gyro_type gyro2(0, 0, 0);
        imu_database_type::imu_sample_type::accel_type accel2(0, 0, 0);
        accel2(i, 0) = 1;

        const imu_database_type::imu_sample_type imu1(99, gyro1, accel1);
        const imu_database_type::imu_sample_type imu2(101, gyro2, accel2);

        imuDatabase.addImuSample(imu1);
        imuDatabase.addImuSample(imu2);

        ode_type ode(y0, t0, gravity, imuDatabase);

        RungeKutta4thOrderIntegrator<double, StateODE<double> > integrator;
        const ode_type::y_type y = integrator.integrate(ode, 1);

        EXPECT_DOUBLE_EQ(1, y(6 + i, 0));
        EXPECT_DOUBLE_EQ(0.5, y(12 + i, 0));
    }
}

TEST(ImuPropagator, gyroXState) {
    typedef ImuPropagator<double, double> propagator_type;

    const double oneDegreeInRadians = 1 * M_PI / 180;

    const propagator_type::noise_type gyroWhiteNoise(0, 0, 0);
    const propagator_type::noise_type gyroBiasNoise(0, 0, 0);
    const propagator_type::noise_type accelWhiteNoise(0, 0, 0);
    const propagator_type::noise_type accelBiasNoise(0, 0, 0);

    const details::gravity_type<double> gravity(0, 0, 0);

    propagator_type::imu_database_type imuDatabase(3);

    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro1(oneDegreeInRadians, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel1(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro2(oneDegreeInRadians, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel2(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro3(oneDegreeInRadians, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel3(0, 0, 0);
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(99, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(100, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(101, gyro1, accel1));

    propagator_type propagator(gyroWhiteNoise, gyroBiasNoise, accelWhiteNoise, accelBiasNoise, gravity);

    details::state_type<double> state = details::state_type<double>::Zero();

    const propagator_type::covariance_ode_type::y_type stateCov = propagator_type::covariance_ode_type::y_type::Zero();
    const details::state_type<double> nextState = propagator.propagate(99, state, stateCov, imuDatabase, 100);

    EXPECT_NEAR(oneDegreeInRadians, 2 * nextState(0, 0), 1e-4);

    for (size_t i = 1; i < StateODE<double>::y_rows; i++) {
        EXPECT_EQ(0, nextState(i, 0));
    }
}

TEST(ImuPropagator, gyroYState) {
    typedef ImuPropagator<double, double> propagator_type;

    const double oneDegreeInRadians = 1 * M_PI / 180;

    const propagator_type::noise_type gyroWhiteNoise(0, 0, 0);
    const propagator_type::noise_type gyroBiasNoise(0, 0, 0);
    const propagator_type::noise_type accelWhiteNoise(0, 0, 0);
    const propagator_type::noise_type accelBiasNoise(0, 0, 0);

    const details::gravity_type<double> gravity(0, 0, 0);

    propagator_type::imu_database_type imuDatabase(3);

    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro1(0, oneDegreeInRadians, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel1(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro2(0, oneDegreeInRadians, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel2(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro3(0, oneDegreeInRadians, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel3(0, 0, 0);
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(99, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(100, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(101, gyro1, accel1));

    propagator_type propagator(gyroWhiteNoise, gyroBiasNoise, accelWhiteNoise, accelBiasNoise, gravity);

    details::state_type<double> state = details::state_type<double>::Zero();

    const propagator_type::covariance_ode_type::y_type stateCov = propagator_type::covariance_ode_type::y_type::Zero();
    const details::state_type<double> nextState = propagator.propagate(99, state, stateCov, imuDatabase, 100);

    EXPECT_EQ(0, nextState(0, 0));
    EXPECT_NEAR(oneDegreeInRadians, 2 * nextState(1, 0), 1e-4);

    for (size_t i = 2; i < StateODE<double>::y_rows; i++) {
        EXPECT_EQ(0, nextState(i, 0));
    }
}

TEST(ImuPropagator, gyroZState) {
    typedef ImuPropagator<double, double> propagator_type;

    const double oneDegreeInRadians = 1 * M_PI / 180;

    const propagator_type::noise_type gyroWhiteNoise(0, 0, 0);
    const propagator_type::noise_type gyroBiasNoise(0, 0, 0);
    const propagator_type::noise_type accelWhiteNoise(0, 0, 0);
    const propagator_type::noise_type accelBiasNoise(0, 0, 0);

    const details::gravity_type<double> gravity(0, 0, 0);

    propagator_type::imu_database_type imuDatabase(3);

    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro1(0, 0, oneDegreeInRadians);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel1(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro2(0, 0, oneDegreeInRadians);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel2(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro3(0, 0, oneDegreeInRadians);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel3(0, 0, 0);
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(99, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(100, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(101, gyro1, accel1));

    propagator_type propagator(gyroWhiteNoise, gyroBiasNoise, accelWhiteNoise, accelBiasNoise, gravity);

    details::state_type<double> state = details::state_type<double>::Zero();

    const propagator_type::covariance_ode_type::y_type stateCov = propagator_type::covariance_ode_type::y_type::Zero();
    const details::state_type<double> nextState = propagator.propagate(99, state, stateCov, imuDatabase, 100);

    EXPECT_EQ(0, nextState(0, 0));
    EXPECT_EQ(0, nextState(1, 0));
    EXPECT_NEAR(oneDegreeInRadians, 2 * nextState(2, 0), 1e-4);

    for (size_t i = 3; i < StateODE<double>::y_rows; i++) {
        EXPECT_EQ(0, nextState(i, 0));
    }
}

TEST(ImuPropagator, accelXState) {
    typedef ImuPropagator<double, double> propagator_type;

    const propagator_type::noise_type gyroWhiteNoise(0, 0, 0);
    const propagator_type::noise_type gyroBiasNoise(0, 0, 0);
    const propagator_type::noise_type accelWhiteNoise(0, 0, 0);
    const propagator_type::noise_type accelBiasNoise(0, 0, 0);

    const details::gravity_type<double> gravity(0, 0, 0);

    propagator_type::imu_database_type imuDatabase(3);

    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro1(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel1(1, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro2(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel2(1, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro3(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel3(1, 0, 0);
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(99, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(100, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(101, gyro1, accel1));

    propagator_type propagator(gyroWhiteNoise, gyroBiasNoise, accelWhiteNoise, accelBiasNoise, gravity);

    details::state_type<double> state = details::state_type<double>::Zero();

    const propagator_type::covariance_ode_type::y_type stateCov = propagator_type::covariance_ode_type::y_type::Zero();
    const details::state_type<double> nextState = propagator.propagate(99, state, stateCov, imuDatabase, 100);

    EXPECT_DOUBLE_EQ(1, nextState(6, 0));
    EXPECT_DOUBLE_EQ(0.5, nextState(12, 0));
}

TEST(ImuPropagator, accelYState) {
    typedef ImuPropagator<double, double> propagator_type;

    const propagator_type::noise_type gyroWhiteNoise(0, 0, 0);
    const propagator_type::noise_type gyroBiasNoise(0, 0, 0);
    const propagator_type::noise_type accelWhiteNoise(0, 0, 0);
    const propagator_type::noise_type accelBiasNoise(0, 0, 0);
    const details::gravity_type<double> gravity(0, 0, 0);

    propagator_type::imu_database_type imuDatabase(3);

    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro1(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel1(0, 1, 0);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro2(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel2(0, 1, 0);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro3(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel3(0, 1, 0);
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(99, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(100, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(101, gyro1, accel1));

    propagator_type propagator(gyroWhiteNoise, gyroBiasNoise, accelWhiteNoise, accelBiasNoise, gravity);

    details::state_type<double> state = details::state_type<double>::Zero();

    const propagator_type::covariance_ode_type::y_type stateCov = propagator_type::covariance_ode_type::y_type::Zero();
    const details::state_type<double> nextState = propagator.propagate(99, state, stateCov, imuDatabase, 100);

    EXPECT_DOUBLE_EQ(1, nextState(7, 0));
    EXPECT_DOUBLE_EQ(0.5, nextState(13, 0));
}

TEST(ImuPropagator, accelZState) {
    typedef ImuPropagator<double, double> propagator_type;

    const propagator_type::noise_type gyroWhiteNoise(0, 0, 0);
    const propagator_type::noise_type gyroBiasNoise(0, 0, 0);
    const propagator_type::noise_type accelWhiteNoise(0, 0, 0);
    const propagator_type::noise_type accelBiasNoise(0, 0, 0);

    const details::gravity_type<double> gravity(0, 0, 0);

    propagator_type::imu_database_type imuDatabase(3);

    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro1(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel1(0, 0, 1);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro2(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel2(0, 0, 1);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro3(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel3(0, 0, 1);
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(99, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(100, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(101, gyro1, accel1));

    propagator_type propagator(gyroWhiteNoise, gyroBiasNoise, accelWhiteNoise, accelBiasNoise, gravity);

    details::state_type<double> state = details::state_type<double>::Zero();

    const propagator_type::covariance_ode_type::y_type stateCov = propagator_type::covariance_ode_type::y_type::Zero();
    const details::state_type<double> nextState = propagator.propagate(99, state, stateCov, imuDatabase, 100);

    EXPECT_DOUBLE_EQ(1, nextState(8, 0));
    EXPECT_DOUBLE_EQ(0.5, nextState(14, 0));
}

TEST(ImuPropagator, jacobian) {
    typedef ImuPropagator<double, double> propagator_type;

    const propagator_type::noise_type gyroWhiteNoise(0, 0, 0);
    const propagator_type::noise_type gyroBiasNoise(0, 0, 0);
    const propagator_type::noise_type accelWhiteNoise(0, 0, 0);
    const propagator_type::noise_type accelBiasNoise(0, 0, 0);

    const details::gravity_type<double> gravity(0, 0, 0);

    propagator_type::imu_database_type imuDatabase(3);

    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro1(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel1(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro2(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel2(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::gyro_type gyro3(0, 0, 0);
    propagator_type::imu_database_type::imu_sample_type::accel_type accel3(0, 0, 0);
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(99, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(99.1, gyro1, accel1));
    imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(99.2, gyro1, accel1));

    propagator_type propagator(gyroWhiteNoise, gyroBiasNoise, accelWhiteNoise, accelBiasNoise, gravity);

    details::state_type<double> state = details::state_type<double>::Zero();

    Eigen::Vector3d axis(1, -2, 3);
    axis.normalize();
    const Eigen::AngleAxis<double> axisAngle(5 * M_PI / 180, axis);
    const Eigen::Quaternion<double> q(axisAngle);
    state.block(0, 0, 3, 1) = q.vec();
    state.block(3, 0, 3, 1) = Eigen::Vector3d(-1 * M_PI / 180, 2 * M_PI / 180, -3 * M_PI / 180);
    state.block(6, 0, 3, 1) = Eigen::Vector3d(1, -2, 3);
    state.block(9, 0, 3, 1) = Eigen::Vector3d(-1, 2, -3);
    state.block(12, 0, 3, 1) = Eigen::Vector3d(100, -200, 300);

    details::state_type<double> perturbedState = state;
    for (size_t i = 0; i < 15; i++) {
        perturbedState(i, 0) += 0.01 * (i % 2 == 0);
    }

    const propagator_type::covariance_ode_type::y_type stateCov = propagator_type::covariance_ode_type::y_type::Zero();
    const details::state_type<double> nextState = propagator.propagate(99, state, stateCov, imuDatabase, 99.1);
    const details::state_type<double> perturbedNextState = propagator.propagate(99, perturbedState, stateCov, imuDatabase, 99.1);

    const details::state_type<double> errorState = PropagationEquations<double>::residual(perturbedState, state);
    const details::state_type<double> errorNextState = PropagationEquations<double>::residual(perturbedNextState, nextState);

    const details::state_type<double> propagatedErrorState = propagator.jacobian() * errorState;

    for (size_t i = 0; i < 15; i++) {
        EXPECT_NEAR(errorNextState(i, 0), propagatedErrorState(i, 0), 1e-3) << " at position: " << i;
    }
}

TEST(ImuPropagator, gyroCovariance) {
    typedef ImuPropagator<double, double> propagator_type;

    const double oneDegreeInRadians = 1 * M_PI / 180;
    const propagator_type::noise_type gyroBiasNoise(0, 0, 0);
    const propagator_type::noise_type accelWhiteNoise(0, 0, 0);
    const propagator_type::noise_type accelBiasNoise(0, 0, 0);
    const details::gravity_type<double> gravity(0, 0, 0);

    for (size_t i = 0; i < 3; i++) {
        propagator_type::noise_type gyroWhiteNoise(0, 0, 0);
        gyroWhiteNoise(i, 0) = oneDegreeInRadians;

        propagator_type::imu_database_type imuDatabase(3);

        propagator_type::imu_database_type::imu_sample_type::gyro_type gyro1(0, 0, 0);
        propagator_type::imu_database_type::imu_sample_type::accel_type accel1(0, 0, 0);
        propagator_type::imu_database_type::imu_sample_type::gyro_type gyro2(0, 0, 0);
        propagator_type::imu_database_type::imu_sample_type::accel_type accel2(0, 0, 0);
        propagator_type::imu_database_type::imu_sample_type::gyro_type gyro3(0, 0, 0);
        propagator_type::imu_database_type::imu_sample_type::accel_type accel3(0, 0, 0);
        imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(99, gyro1, accel1));
        imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(100, gyro1, accel1));
        imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(101, gyro1, accel1));

        propagator_type propagator(gyroWhiteNoise, gyroBiasNoise, accelWhiteNoise, accelBiasNoise, gravity);

        details::state_type<double> state = details::state_type<double>::Zero();

        Eigen::Vector3d axis(1, -2, 3);
        axis.normalize();
        const Eigen::AngleAxis<double> axisAngle(5 * M_PI / 180, axis);
        const Eigen::Quaternion<double> q(axisAngle);
        state.block(0, 0, 3, 1) = q.vec();
        state.block(3, 0, 3, 1) = Eigen::Vector3d(-1 * M_PI / 180, 2 * M_PI / 180, -3 * M_PI / 180);
        state.block(6, 0, 3, 1) = Eigen::Vector3d(1, -2, 3);
        state.block(9, 0, 3, 1) = Eigen::Vector3d(-1, 2, -3);
        state.block(12, 0, 3, 1) = Eigen::Vector3d(100, -200, 300);

        const propagator_type::covariance_ode_type::y_type stateCov = propagator_type::covariance_ode_type::y_type::Zero();
        propagator.propagate(99, state, stateCov, imuDatabase, 100);

        EXPECT_NEAR(oneDegreeInRadians * oneDegreeInRadians, propagator.covariance()(i, i), 1e-6);
    }
}

TEST(ImuPropagator, positionCovariance) {
    typedef ImuPropagator<double, double> propagator_type;

    const double accelWhiteNoiseValue = 1;

    const propagator_type::noise_type gyroWhiteNoise(0, 0, 0);
    const propagator_type::noise_type gyroBiasNoise(0, 0, 0);
    const propagator_type::noise_type accelBiasNoise(0, 0, 0);
    const details::gravity_type<double> gravity(0, 0, 0);

    for (size_t i = 0; i < 3; i++) {
        propagator_type::noise_type accelWhiteNoise(0, 0, 0);
        accelWhiteNoise(i, 0) = accelWhiteNoiseValue;

        propagator_type::imu_database_type imuDatabase(3);

        propagator_type::imu_database_type::imu_sample_type::gyro_type gyro1(0, 0, 0);
        propagator_type::imu_database_type::imu_sample_type::accel_type accel1(0, 0, 0);
        propagator_type::imu_database_type::imu_sample_type::gyro_type gyro2(0, 0, 0);
        propagator_type::imu_database_type::imu_sample_type::accel_type accel2(0, 0, 0);
        propagator_type::imu_database_type::imu_sample_type::gyro_type gyro3(0, 0, 0);
        propagator_type::imu_database_type::imu_sample_type::accel_type accel3(0, 0, 0);
        imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(99, gyro1, accel1));
        imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(100, gyro1, accel1));
        imuDatabase.addImuSample(propagator_type::imu_database_type::imu_sample_type(101, gyro1, accel1));

        propagator_type propagator(gyroWhiteNoise, gyroBiasNoise, accelWhiteNoise, accelBiasNoise, gravity);

        details::state_type<double> state = details::state_type<double>::Zero();

        const propagator_type::covariance_ode_type::y_type stateCov = propagator_type::covariance_ode_type::y_type::Zero();
        propagator.propagate(99, state, stateCov, imuDatabase, 100);

        EXPECT_DOUBLE_EQ(1, propagator.covariance()(6 + i, 6 + i));
        EXPECT_DOUBLE_EQ(1.0 / 3.0, propagator.covariance()(12 + i, 12 + i));
        EXPECT_DOUBLE_EQ(0.5, propagator.covariance()(12 + i, 6 + i));
        EXPECT_DOUBLE_EQ(0.5, propagator.covariance()(6 + i, 12 + i));
    }
}

TEST(imaginaryToQuaternion, validQuaternion) {
    const Eigen::Matrix<double, 3, 1> imaginary = { 0.1, 0.2, 0.3 };
    const Eigen::Quaternion<double> q = imaginaryToQuaternion(imaginary);

    EXPECT_EQ(0.1, q.x());
    EXPECT_EQ(0.2, q.y());
    EXPECT_EQ(0.3, q.z());
    EXPECT_NEAR(0.92736184955, q.w(), 1e-10);
}
