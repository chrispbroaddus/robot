import numpy as np
from rigidbody import normalized, SE3, rotation, pr, unpr

from . import interpolate
from ..state import InertialState
from .functions import StateFunction, PoseFunction


def truncate_vector(x, norm):
    """
    Return a vector in the direction of x with length no greater than NORM
    """
    a = np.linalg.norm(x)
    if a > norm:
        return x * norm / a
    else:
        return x


def sample_random_walk(timestamps, sigma, initial_norm, max_norm, ndims=3):
    """
    Sample from a random walk process in N dimensions
    """
    if len(timestamps) == 0:
        return []
    values = [truncate_vector(normalized(np.random.randn(ndims)) * initial_norm, max_norm)]
    for i in range(1, len(timestamps)):
        dt = (timestamps[i] - timestamps[i - 1])
        delta = np.random.randn(ndims) * sigma * dt
        values.append(truncate_vector(values[-1] + delta, max_norm))
    return values


def generate_positions_random_walk(duration, velocity_excitation=.5, initial_speed=1.,
                                   max_speed=1., control_frequency=4., **kwargs):
    timestamps = np.arange(0, duration, 1. / control_frequency)
    velocities = sample_random_walk(
        timestamps,
        velocity_excitation,
        initial_speed,
        max_speed)

    positions = [np.zeros(3)]
    for i in range(1, len(timestamps)):
        dt = timestamps[i] - timestamps[i - 1]
        positions.append(positions[-1] + dt * velocities[i - 1])

    return timestamps, positions


def generate_positions_random_walk_2d(duration, **kwargs):
    ts, ps = generate_positions_random_walk(duration, **kwargs)
    ps = np.asarray(ps)
    ps[:, 2] = 0
    return ts, ps


def generate_positions_circle_2d(duration, radius=1., **kwargs):
    ts = np.linspace(0, duration, 100)
    ps = np.transpose([np.sin(ts)*radius, np.cos(ts)*radius, np.zeros(len(ts))])
    return ts, ps


def generate_positions_line(duration, **kwargs):
    ts = np.linspace(0, duration, 10)
    velocity = np.random.rand(3)
    ps = np.outer(ts, velocity)
    return ts, ps


def generate_positions_manhattan_2d(duration,
                                    control_frequency=4.,
                                    mean_velocity=1.,
                                    sigma_velocity=.1,
                                    min_segment_length=3.,
                                    max_segment_length=10.,
                                    turning_radius=.5,
                                    **kwargs):
    # Sample a velocity (will remain constant throughout)
    speed = mean_velocity + np.random.randn() * sigma_velocity

    min_segment_duration = min_segment_length / speed
    max_segment_duration = max_segment_length / speed
    turning_duration = turning_radius / speed

    assert min_segment_duration > turning_duration * 2

    # First construct corners
    corner_timestamps = [0.]
    corner_positions = [np.zeros(3)]
    cur_axis = 0
    while corner_timestamps[-1] < duration:
        # Pick a direction
        velocity = np.zeros(3)
        velocity[cur_axis] = speed * (-1 if np.random.rand() < .5 else 1)

        # Sample next timestamp
        segment_duration = np.random.uniform(min_segment_duration, max_segment_duration)
        next_timestamp = corner_timestamps[-1] + segment_duration
        if next_timestamp > duration:
            next_timestamp = duration

        # Sample next position
        next_position = corner_positions[-1] + segment_duration * velocity

        # Add to lists
        corner_timestamps.append(next_timestamp)
        corner_positions.append(next_position)

        cur_axis = 1 - cur_axis

    # Now construct control points by interpolating between corners
    timestamps = []
    positions = []
    for i in range(len(corner_timestamps) - 1):
        p0, p1 = corner_positions[i], corner_positions[i+1]
        t0, t1 = corner_timestamps[i], corner_timestamps[i+1]
        first_ctrl, last_ctrl = corner_timestamps[i], corner_timestamps[i+1]

        if i > 0:
            first_ctrl += turning_duration

        if i < len(corner_timestamps) - 2:
            last_ctrl -= turning_duration

        assert last_ctrl > first_ctrl
        num_controls = np.ceil((last_ctrl - first_ctrl) * control_frequency) + 1

        for t in np.linspace(first_ctrl, last_ctrl, num_controls):
            timestamps.append(t)
            positions.append(p0 + (t - t0) * (p1 - p0) / (t1 - t0))

    return timestamps, positions


def generate_positions_square_2d(duration, speed=1., side_length=5., stationary_duration=0., **kwargs):
    corners = np.array([
        [0., 0., 0.],
        [side_length, 0., 0.],
        [side_length, side_length, 0.],
        [0., side_length, 0.]
    ])
    cur_index = 0
    cur_timestamp = 0.
    timestamps = [0]
    positions = [corners[0]]
    while cur_timestamp <= duration:
        cur_index = (cur_index + 1) % len(corners)
        cur_timestamp += side_length / speed
        timestamps.append(cur_timestamp)
        positions.append(corners[cur_index])
        if stationary_duration > 0:
            cur_timestamp += stationary_duration
            timestamps.append(cur_timestamp)
            positions.append(corners[cur_index])
    return interpolate.interpolate_piecewise_straight(timestamps, positions)


def generate_orientations_random_walk(position_curve, duration, control_frequency,
                                      excitation, initial_speed, max_speed, **kwargs):
    timestamps = np.arange(0, duration, 1. / control_frequency)
    angular_velocities = sample_random_walk(
        timestamps,
        excitation,
        initial_speed,
        max_speed)
    orientations = [np.eye(3)]
    for i in range(1, len(timestamps)):
        dt = timestamps[i] - timestamps[i - 1]
        dr = rotation.exp(dt * angular_velocities[i - 1])
        orientations.append(np.dot(orientations[-1], dr))
    return timestamps, orientations


def generate_orientations_forward_facing(position_curve, duration, control_frequency=5., **kwargs):
    timestamps = []
    orientations = []
    velocity_curve = position_curve.derivative(1)
    ez = np.array((0, 0, 1))
    for t in np.arange(0, duration, 1. / control_frequency):
        velocity = velocity_curve(t)
        speed = np.linalg.norm(velocity)
        # If speed is small then do not create a control point at this position
        if speed > 1e-3:
            heading = velocity / speed
            r = np.zeros((3, 3))
            r[2] = heading
            r[1] = normalized(ez - np.dot(ez, heading) * heading)
            r[0] = np.cross(r[1], r[2])
            # noinspection PyTypeChecker
            if np.linalg.det(r) < 0:
                r[0] = -r[0]
            timestamps.append(t)
            orientations.append(r)
    return timestamps, orientations


def generate_gyro_bias_random_walk(trajectory, duration, gyro_bias_sigma, control_frequency=4.,
                                   initial_gyro_bias_norm=0., max_gyro_bias_norm=.1, **kwargs):
    timestamps = np.arange(0, duration, 1. / control_frequency)
    gyro_biases = sample_random_walk(
        timestamps,
        gyro_bias_sigma,
        initial_gyro_bias_norm,
        max_gyro_bias_norm)
    return timestamps, gyro_biases


def generate_accel_bias_random_walk(trajectory, duration, accel_bias_sigma, control_frequency=4.,
                                    initial_accel_bias_norm=0., max_accel_bias_norm=.1, **kwargs):
    timestamps = np.arange(0, duration, 1. / control_frequency)
    accel_biases = sample_random_walk(
        timestamps,
        accel_bias_sigma,
        initial_accel_bias_norm,
        max_accel_bias_norm)
    return timestamps, accel_biases


def generate_state_function(position_generator, orientation_generator, gravity_norm=9.8, **kwargs):
    # Generate position curve
    print('Generating position controls...')
    if position_generator == 'random_walk':
        position_timestamps, positions = generate_positions_random_walk(**kwargs)
        position_curve = interpolate.interpolate_spline(position_timestamps, positions)
    elif position_generator == 'random_walk_2d':
        position_timestamps, positions = generate_positions_random_walk_2d(**kwargs)
        position_curve = interpolate.interpolate_spline(position_timestamps, positions)
    elif position_generator == 'manhattan_2d':
        position_timestamps, positions = generate_positions_manhattan_2d(**kwargs)
        position_curve = interpolate.interpolate_spline(position_timestamps, positions)
    elif position_generator == 'line':
        position_timestamps, positions = generate_positions_line(**kwargs)
        position_curve = interpolate.interpolate_spline(position_timestamps, positions)
    elif position_generator == 'circle_2d':
        position_timestamps, positions = generate_positions_circle_2d(**kwargs)
        position_curve = interpolate.interpolate_spline(position_timestamps, positions)
    elif position_generator == 'square_2d':
        position_curve = generate_positions_square_2d(**kwargs)
    else:
        raise ValueError('unrecognized position generator: %s' % position_generator)

    # Construct orientation curve
    print('Sampling orientation controls...')
    if orientation_generator == 'random_walk':
        orient_timestamps, orients = generate_orientations_random_walk(position_curve, **kwargs)
        orientation_curve = interpolate.interpolate_orientation_spline(orient_timestamps, orients)
    elif orientation_generator == 'forward_facing':
        orient_timestamps, orients = generate_orientations_forward_facing(position_curve, **kwargs)
        orientation_curve = interpolate.interpolate_orientation_spline(orient_timestamps, orients)
    else:
        raise ValueError('unrecognized orientation generator: %s' % orientation_generator)

    # Construct trajectory
    trajectory = PoseFunction(position_curve, orientation_curve)

    # Construct gravity vector
    gravity = np.array((0., 0., gravity_norm))

    # Generate gyro bias
    gyro_bias_timestamps, gyro_biases = generate_gyro_bias_random_walk(trajectory, **kwargs)
    assert np.all(np.diff(gyro_bias_timestamps) > 0), 'gyro bias timestamps were out of order'
    gyro_bias_curve = interpolate.interpolate_spline(gyro_bias_timestamps, gyro_biases)

    # Generate accel bias
    accel_bias_timestamps, accel_biases = generate_accel_bias_random_walk(trajectory, **kwargs)
    assert np.all(np.diff(accel_bias_timestamps) > 0), 'accel bias timestamps were out of order'
    accel_bias_curve = interpolate.interpolate_spline(accel_bias_timestamps, accel_biases)

    # Construct full state function
    return StateFunction(trajectory, gyro_bias_curve, accel_bias_curve, gravity)
