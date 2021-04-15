import unittest
import numpy as np
from numericaltesting import assert_arrays_almost_equal
from manifolds import numeric_jacobian

from . import integration
from .models import ImuMeasurement, InertialSensorModel
from .state import InertialState
from . import simulator


inertial_model = InertialSensorModel(gyro_sigma=1e-2,
                                     gyro_bias_sigma=1e-2,
                                     accel_sigma=1e-2,
                                     accel_bias_sigma=1e-2)


class IntegratorTest(unittest.TestCase):
    def setUp(self):
        np.set_printoptions(suppress=True, linewidth=500)

    def test_integrate_state(self):
        np.random.seed(0)

        state_func = simulator.generate_state_function(position_generator='random_walk',
                                                       orientation_generator='forward_facing',
                                                       duration=5.,
                                                       gyro_bias_sigma=0,
                                                       accel_bias_sigma=0,
                                                       initial_gyro_bias_norm=1.,
                                                       initial_accel_bias_norm=1.)

        duration = .5
        imu_measurements = ImuMeasurement.List()
        for t in np.linspace(0, duration, 100):
            imu_measurements.append_new(timestamp=t,
                                        gyro_reading=state_func.gyro(t),
                                        accel_reading=state_func.accelerometer(t))

        p = integration.RK4Integrator(inertial_model)

        begin_state = state_func(0.)
        integrated_state = p.integrate_state(begin_state, state_func.gravity, imu_measurements)
        expected_state = state_func(duration)

        assert_arrays_almost_equal(expected_state.position, integrated_state.position, decimals=4)
        assert_arrays_almost_equal(expected_state.velocity, integrated_state.velocity, decimals=4)
        assert_arrays_almost_equal(expected_state.orientation, integrated_state.orientation, decimals=4)
        assert_arrays_almost_equal(expected_state.gyro_bias, integrated_state.gyro_bias)
        assert_arrays_almost_equal(expected_state.accel_bias, integrated_state.accel_bias)

    def test_integrate_noise(self):
        np.random.seed(0)

        state_func = simulator.generate_state_function(position_generator='random_walk',
                                                        orientation_generator='forward_facing',
                                                        duration=5.,
                                                        gyro_bias_sigma=0,
                                                        accel_bias_sigma=0,
                                                        initial_gyro_bias_norm=1.,
                                                        initial_accel_bias_norm=1.)

        duration = 5.
        imu_measurements = ImuMeasurement.List()
        for t in np.linspace(0, duration, 100):
            imu_measurements.append_new(timestamp=t,
                                        gyro_reading=state_func.gyro(t),
                                        accel_reading=state_func.accelerometer(t))

        p = integration.RK4Integrator(inertial_model)

        begin_state = state_func(0.)
        result = p.integrate_state_and_jacobian(begin_state, state_func.gravity, imu_measurements)

        #print result.covariance
        print('velocity uncertainty:', np.diag(result.covariance[6:9,6:9]))
        print('position uncertainty:', np.diag(result.covariance[-3:,-3:]))

    def test_time_derivative(self):
        np.random.seed(0)

        # Noise levels used to generate simulated data
        state_func = simulator.generate_state_function(position_generator='circle_2d',
                                                        orientation_generator='forward_facing',
                                                        duration=5.,
                                                        gyro_bias_sigma=0,
                                                        accel_bias_sigma=0,
                                                        initial_gyro_bias_norm=1.,
                                                        initial_accel_bias_norm=1.)

        t = 2.3
        state = state_func(t)
        gyro = state_func.gyro(t)
        accel = state_func.accelerometer(t)
        j_analytic = integration.compute_time_derivative(state, state_func.gravity, gyro, accel)
        j_numeric = np.squeeze(numeric_jacobian(state_func, t))

        assert_arrays_almost_equal(j_analytic, j_numeric)

    def test_continuous_time_transition_matrix(self):
        np.random.seed(0)

        state = InertialState.from_tangent(np.random.rand(InertialState.DoF))
        gyro = np.random.randn(3)
        accel = np.random.randn(3)
        gravity = np.random.randn(3)

        phi_analytic = integration.compute_continuous_time_transition(state, gyro, accel)

        f = lambda state: integration.compute_time_derivative(state, gravity, gyro, accel)
        phi_numeric = numeric_jacobian(f, state)

        assert_arrays_almost_equal(phi_analytic, phi_numeric)

    def test_continuous_time_noise_transfer_matrix(self):
        np.random.seed(0)

        state = InertialState.from_tangent(np.random.rand(InertialState.DoF))
        gyro = np.random.randn(3)
        accel = np.random.randn(3)
        gravity = np.random.randn(3)

        j_wrt_gyro, j_wrt_accel, j_wrt_gyro_bias, j_wrt_accel_bias = \
            integration.compute_continuous_time_noise_transfer(state, gyro, accel)

        j_wrt_gyro_numeric = numeric_jacobian(
            lambda x: integration.compute_time_derivative(state, gravity, x, accel), gyro)
        j_wrt_accel_numeric = numeric_jacobian(
            lambda x: integration.compute_time_derivative(state, gravity, gyro, x), accel)

        assert_arrays_almost_equal(j_wrt_gyro, j_wrt_gyro_numeric)
        assert_arrays_almost_equal(j_wrt_accel, j_wrt_accel_numeric)

    def test_discrete_time_transition_matrix(self):
        np.random.seed(0)

        gravity = np.random.randn(3)
        imu_measurements = ImuMeasurement.List()
        imu_measurements.append_new(timestamp=0., gyro_reading=np.random.randn(3), accel_reading=np.random.randn(3))
        imu_measurements.append_new(timestamp=1., gyro_reading=np.random.randn(3), accel_reading=np.random.randn(3))

        f = lambda state: integration.integrate_state_rk4(
            state,
            gravity,
            1.,
            imu_measurements[0].gyro_reading,
            imu_measurements[1].gyro_reading,
            imu_measurements[0].accel_reading,
            imu_measurements[1].accel_reading)

        begin_state = InertialState.from_tangent(np.random.rand(InertialState.DoF))
        final_state, j_analytic = f(begin_state)

        j_numeric = numeric_jacobian(lambda state: f(state)[0], begin_state)

        assert_arrays_almost_equal(j_analytic, j_numeric)

    def test_propagator_jacobian(self):
        np.random.seed(0)

        state_func = simulator.generate_state_function(position_generator='random_walk',
                                                        orientation_generator='forward_facing',
                                                        duration=5.,
                                                        gyro_bias_sigma=0,
                                                        accel_bias_sigma=0,
                                                        initial_gyro_bias_norm=1.,
                                                        initial_accel_bias_norm=1.)

        begin_time = 0.
        duration = .5
        imu_measurements = ImuMeasurement.List()
        for t in np.linspace(begin_time, duration, 10):
            imu_measurements.append_new(timestamp=t,
                                        gyro_reading=state_func.gyro(t),
                                        accel_reading=state_func.accelerometer(t))

        p = integration.RK4Integrator(inertial_model)

        begin_state = state_func(0)
        result = p.integrate_state_and_jacobian(begin_state, state_func.gravity, imu_measurements)

        f = lambda state: p.integrate_state(state, state_func.gravity, imu_measurements)
        j_numeric = numeric_jacobian(f, begin_state)

        assert_arrays_almost_equal(result.jacobian, j_numeric)
