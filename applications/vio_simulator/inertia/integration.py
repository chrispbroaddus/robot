import numpy as np
from rigidbody import rotation
from mutablerecord import MutableRecordType, Required

from .state import InertialState


class IntegratorResult(object, metaclass=MutableRecordType):
    """
    Represents the result of one call to the integrator.
    """
    state = Required
    jacobian = Required
    covariance = Required


def compute_time_derivative(state, gravity, gyro, accel):
    """
    Compute the time derivative of the state given accel and gyro readings.
    """
    assert gravity.shape == (3,), 'shape was %s' % str(gravity.shape)
    assert gyro.shape == (3,), 'shape was %s' % str(gyro.shape)
    assert accel.shape == (3,), 'shape was %s' % str(accel.shape)
    j = np.zeros(InertialState.DoF)
    # derivative of orientation with respect to time (left chart)
    j[:3] = state.gyro_bias - gyro
    # derivative of velocity with respect to time
    j[6:9] = np.dot(state.pose.orientation.T, accel - state.accel_bias) - gravity
    # derivative of position with respect to time
    j[12:] = state.velocity
    return j


def compute_continuous_time_transition(state, gyro, accel):
    """
    Compute the continuous time transition matrix given accel and gyro readings. The continuous 
    time transition matrix is the derivative of the time derivative with respect to the state.
    """
    j = np.zeros((InertialState.DoF, InertialState.DoF))
    # derivative of orientation with respect to gyro bias (in left chart)
    j[:3, 3:6] = np.eye(3)
    # derivative of velocity with respect to orientation
    j[6:9, :3] = -np.dot(state.pose.orientation.T, rotation.skew(state.accel_bias - accel))
    # derivative of velocity with respect to accel bias
    j[6:9, 9:12] = -state.pose.orientation.T
    # derivative of position with respect to velocity
    j[12:, 6:9] = np.eye(3)
    return j


def compute_continuous_time_noise_transfer(state, gyro, accel):
    """
    Compute the derivative of the time derivative with respect to:
    [ gyro_noise, gyro_bias_noise, accel_noise, accel_bias_noise ].
    """
    # partial derivatives w.r.t. gyro reading
    j_wrt_gyro = np.vstack((-np.eye(3), np.zeros((12, 3))))
    # partial derivatives w.r.t. accel reading
    j_wrt_accel = np.vstack((np.zeros((6, 3)), state.pose.orientation.T, np.zeros((6, 3))))
    # partial derivatives w.r.t. gyro bias
    j_wrt_gyro_bias = np.vstack((np.zeros((3, 3)), np.eye(3), np.zeros((9, 3))))
    # partial derivatives w.r.t. accel reading
    j_wrt_accel_bias = np.vstack((np.zeros((9, 3)), np.eye(3), np.zeros((3, 3))))
    return j_wrt_gyro, j_wrt_accel, j_wrt_gyro_bias, j_wrt_accel_bias


def compute_noise_derivative(state, gyro, accel, inertial_model):
    """
    Compute the derivative of the covariance with respect to time
    """
    j_wrt_gyro, j_wrt_accel, j_wrt_gyro_bias, j_wrt_accel_bias = \
        compute_continuous_time_noise_transfer(state, gyro, accel)

    gyro_term = inertial_model.gyro_sigma * np.dot(j_wrt_gyro, j_wrt_gyro.T)
    accel_term = inertial_model.accel_sigma * np.dot(j_wrt_accel, j_wrt_accel.T)
    gyro_bias_term = inertial_model.gyro_bias_sigma * np.dot(j_wrt_gyro_bias, j_wrt_gyro_bias.T)
    accel_bias_term = inertial_model.accel_bias_sigma * np.dot(j_wrt_accel_bias, j_wrt_accel_bias.T)

    return gyro_term + accel_term + gyro_bias_term + accel_bias_term


def j_perturb_wrt_state(state, tangent):
    """
    Compute the jacobian of perturb(x, delta) with respect to x.
    """
    j = np.eye(InertialState.DoF)
    j[:3, :3] = rotation.exp(tangent[:3])
    return j


def j_perturb_wrt_tangent(state, tangent):
    """
    Compute the jacobian of perturb(x, delta) with respect to x.
    """
    j = np.eye(InertialState.DoF)
    j[:3, :3] = rotation.exp_jacobian(tangent[:3])
    return j


def integrate_state_rk4(begin_state, gravity, duration, begin_gyro, end_gyro, begin_accel, end_accel,
                        with_jacobian=True):
    """
    Given a state at t0 and a pair of measurements at t0 and t1, predict the state
    at t1. If with_jacobian is True then also return the jacobian of the predicted
    state w.r.t. the initial state.
    """

    # def check_jacobian(j_analytic, f, x, atlas=InertialState.Atlas, output_atlas=None):
    #     j_numeric = numeric_jacobian(f, x, atlas=atlas, output_atlas=output_atlas)
    #     assert_array_almost_equal(j_analytic, j_numeric)

    # Convert everything to numpy arrays
    gravity = np.asarray(gravity)
    begin_gyro = np.asarray(begin_gyro)
    end_gyro = np.asarray(end_gyro)
    begin_accel = np.asarray(begin_accel)
    end_accel = np.asarray(end_accel)

    # Interpolate accel and gyro readings
    atlas = InertialState.Atlas
    mid_gyro = (begin_gyro + end_gyro) / 2.
    mid_accel = (begin_accel + end_accel) / 2.

    # Compute sample locations
    d1, d2, d3 = duration*.5, duration*.5, duration

    # Compute intermediate gradients and estimates
    x1 = begin_state
    j1 = compute_time_derivative(x1, gravity, begin_gyro, begin_accel)

    x2 = atlas.perturb(begin_state, j1 * d1)
    j2 = compute_time_derivative(x2, gravity, mid_gyro, mid_accel)

    x3 = atlas.perturb(begin_state, j2 * d2)
    j3 = compute_time_derivative(x3, gravity, mid_gyro, mid_accel)

    x4 = atlas.perturb(begin_state, j3 * d3)
    j4 = compute_time_derivative(x4, gravity, end_gyro, end_accel)

    # Compute weighted average time gradient and final integrated state
    jfinal = (j1 + 2.*j2 + 2.*j3 + j4) / 6.
    xfinal = atlas.perturb(begin_state, duration * jfinal)

    # Should find a more elegant way to do this...
    if not with_jacobian:
        return xfinal

    # Jacobians of each gradient vector with respect to corresponding state
    j_j1_wrt_x1 = compute_continuous_time_transition(x1, begin_gyro, begin_accel)
    j_j2_wrt_x2 = compute_continuous_time_transition(x2, mid_gyro, mid_accel)
    j_j3_wrt_x3 = compute_continuous_time_transition(x3, mid_gyro, mid_accel)
    j_j4_wrt_x4 = compute_continuous_time_transition(x4, end_gyro, end_accel)

    #check_jacobian(j_j1_wrt_x1, lambda yx1: compute_time_derivative(yx1, gravity, begin_gyro, begin_accel), x1)
    #check_jacobian(j_j2_wrt_x2, lambda yx2: compute_time_derivative(yx2, gravity, mid_gyro, mid_accel), x2)
    #check_jacobian(j_j3_wrt_x3, lambda yx3: compute_time_derivative(yx3, gravity, mid_gyro, mid_accel), x3)
    #check_jacobian(j_j4_wrt_x4, lambda yx4: compute_time_derivative(yx4, gravity, end_gyro, end_accel), x4)

    # Jacobians of each state with respect to previous state keeping the gradients j1, j2, j3, j4 fixed
    j_x2_wrt_x1_part = j_perturb_wrt_state(x1, j1 * d1)
    j_x3_wrt_x1_part = j_perturb_wrt_state(x1, j2 * d2)
    j_x4_wrt_x1_part = j_perturb_wrt_state(x1, j3 * d3)

    #check_jacobian(j_x2_wrt_x1_part, lambda x: atlas.perturb(x, j1 * d1), begin_state, output_atlas=atlas)
    #check_jacobian(j_x3_wrt_x1_part, lambda x: atlas.perturb(x, j2 * d2), begin_state, output_atlas=atlas)
    #check_jacobian(j_x4_wrt_x1_part, lambda x: atlas.perturb(x, j3 * d3), begin_state, output_atlas=atlas)

    # Jacobians of each state with respect to gradients j1, j2, j3, j4
    j_x2_wrt_j1 = j_perturb_wrt_tangent(x1, j1 * d1) * d1
    j_x3_wrt_j2 = j_perturb_wrt_tangent(x2, j2 * d2) * d2
    j_x4_wrt_j3 = j_perturb_wrt_tangent(x3, j3 * d3) * d3

    #check_jacobian(j_x2_wrt_j1, lambda k: atlas.perturb(begin_state, k * d1), j1, atlas=None, output_atlas=atlas)
    #check_jacobian(j_x3_wrt_j2, lambda k: atlas.perturb(begin_state, k * d2), j2, atlas=None, output_atlas=atlas)
    #check_jacobian(j_x4_wrt_j3, lambda k: atlas.perturb(begin_state, k * d3), j3, atlas=None, output_atlas=atlas)

    # Jacobian of each gradient j1, j2, j3, j4 with respect to first state
    j_j1_wrt_x1 = j_j1_wrt_x1
    j_j2_wrt_x1 = np.dot(j_j2_wrt_x2, j_x2_wrt_x1_part + np.dot(j_x2_wrt_j1, j_j1_wrt_x1))
    j_j3_wrt_x1 = np.dot(j_j3_wrt_x3, j_x3_wrt_x1_part + np.dot(j_x3_wrt_j2, j_j2_wrt_x1))
    j_j4_wrt_x1 = np.dot(j_j4_wrt_x4, j_x4_wrt_x1_part + np.dot(j_x4_wrt_j3, j_j3_wrt_x1))

    j_jfinal_wrt_x1 = (j_j1_wrt_x1 + 2.*j_j2_wrt_x1 + 2.*j_j3_wrt_x1 + j_j4_wrt_x1) / 6.
    j_xfinal_wrt_jfinal = j_perturb_wrt_tangent(x1, jfinal * duration) * duration
    j_xfinal_wrt_x1_part = j_perturb_wrt_state(x1, jfinal * duration)
    j_xfinal_wrt_x1 = j_xfinal_wrt_x1_part + np.dot(j_xfinal_wrt_jfinal, j_jfinal_wrt_x1)

    return xfinal, j_xfinal_wrt_x1


def integrate_noise_euler(begin_state, duration, gyro, accel, inertial_model):
    """
    Given the state of the device at t0 and measurements received at t0, compute the
    uncertainty in the prediction of the state at t1.
    """
    j_cov_wrt_time = compute_noise_derivative(begin_state, gyro, accel, inertial_model)
    return j_cov_wrt_time * duration


class RK4Integrator(object):
    """
    Integrates the ODE associated with the inertial model.
    """
    def __init__(self, sensor_model):
        self._sensor_model = sensor_model

    def integrate_state(self, state, gravity, imu_measurements):
        for i in range(len(imu_measurements) - 1):
            u0, u1 = imu_measurements[i], imu_measurements[i+1]
            state = integrate_state_rk4(state,
                                        gravity,
                                        u1.timestamp - u0.timestamp,
                                        u0.gyro_reading,
                                        u1.gyro_reading,
                                        u0.accel_reading,
                                        u1.accel_reading,
                                        with_jacobian=False)
        return state

    def integrate_state_and_jacobian(self, state, gravity, imu_measurements):
        j_final_state_wrt_begin_state = np.eye(InertialState.DoF)
        cov = np.zeros((InertialState.DoF, InertialState.DoF))
        for i in range(len(imu_measurements) - 1):
            u0, u1 = imu_measurements[i], imu_measurements[i+1]
            state, j = integrate_state_rk4(state,
                                           gravity,
                                           u1.timestamp - u0.timestamp,
                                           u0.gyro_reading,
                                           u1.gyro_reading,
                                           u0.accel_reading,
                                           u1.accel_reading)
            conditional_cov = integrate_noise_euler(state,
                                                    u1.timestamp - u0.timestamp,
                                                    u0.gyro_reading,
                                                    u0.accel_reading,
                                                    self._sensor_model)
            j_final_state_wrt_begin_state = np.dot(j, j_final_state_wrt_begin_state)
            cov = np.dot(j, np.dot(cov, j.T)) + conditional_cov
        return IntegratorResult(state=state, jacobian=j_final_state_wrt_begin_state, covariance=cov)
