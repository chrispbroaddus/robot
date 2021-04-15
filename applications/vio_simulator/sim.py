import os
import argparse
import tempfile
import subprocess
import numpy as np
from pathlib import Path
from rigidbody import pr
from inertia import ImuMeasurement, InertialSensorModel, RK4Integrator, Camera, simulator
from mutablerecord import MutableRecordType, Required, InstanceOf

import seaborn
from matplotlib import pyplot as plt


class Observables(metaclass=MutableRecordType):
    frames = Required
    gyro_readings = Required
    accel_readings = Required
    frame_timestamps = Required
    gyro_timestamps = Required
    accel_timestamps = Required


class InertialObservables(metaclass=MutableRecordType):
    gyro_readings = Required
    accel_readings = Required
    gyro_timestamps = Required
    accel_timestamps = Required


class Scenario(metaclass=MutableRecordType):
    state_func = Required
    landmarks = Required
    camera = InstanceOf(Camera)
    intrinsic_orientation = np.eye(3)
    intrinsic_displacement = np.zeros(3)


class NoiseModel(metaclass=MutableRecordType):
    inertial = InstanceOf(InertialSensorModel)
    feature_sigma = 0.
    outlier_probability = 0.


class ScenarioOptions(metaclass=MutableRecordType):
    duration = 5.
    control_frequency = 4.
    initial_gyro_bias_norm = 0.
    initial_accel_bias_norm = 0.
    intrinsic_orientation = np.eye(3)
    intrinsic_displacement = np.zeros(3)
    position_generator = "circle_2d"
    orientation_generator = "forward_facing"
    landmark_generator = "cloud"
    landmarks_per_second = 500
    image_xlim = 100
    image_ylim = 100


class ObservableOptions(metaclass=MutableRecordType):
    duration = 5.
    num_frames = 11
    gyro_frequency = 100.
    accel_frequency = 100.


class SeedOptions(metaclass=MutableRecordType):
    num_states = -1
    perturb_first = True
    orientation_perturbation = 0
    position_perturbation = 0
    velocity_perturbation = 0
    gyro_bias_perturbation = 0
    accel_bias_perturbation = 0

class Estimate(metaclass=MutableRecordType):
    states = Required
    gravity = Required
    landmarks = Required


def generate_scenario(options, noise):
    """
    Generate a trajectory and landmarks
    """
    # Check control frequency
    num_controls = options.control_frequency * options.duration
    if num_controls < 5:
        print("Error: there are only %d control points. You may need to increase --control_frequency." % \
            num_controls)
        return

    # Generate device state function
    print('Sampling trajectory...')
    state_func = simulator.generate_state_function(
        gyro_bias_sigma=noise.inertial.gyro_bias_sigma,
        accel_bias_sigma=noise.inertial.accel_bias_sigma,
        **options.dict)
    assert isinstance(state_func, simulator.StateFunction)

    # Generate landmarks
    print('Sampling landmarks...')
    landmarks = simulator.generate_landmarks(state_func, **options.dict)

    # Set up a calibration matrix
    print('Sampling camera calibration...')
    camera = simulator.generate_camera(**options.dict)

    return Scenario(
        state_func=state_func,
        landmarks=landmarks,
        camera=camera,
        intrinsic_orientation=options.intrinsic_orientation,
        intrinsic_displacement=options.intrinsic_displacement)


def generate_observables(options, scenario, noise):
    """
    Generate visual and inertial measurements for a scenario
    """
    # Sample gyro readings
    print('Sampling gyro readings...')
    gyro_timestamps = simulator.generate_timestamps(options.duration, options.gyro_frequency)
    gyro_readings = simulator.generate_gyro_readings(scenario.state_func, gyro_timestamps, noise.inertial.gyro_sigma)

    # Sample accel readings
    print('Sampling accel readings...')
    accel_timestamps = simulator.generate_timestamps(options.duration, options.accel_frequency)
    accel_readings = simulator.generate_accel_readings(scenario.state_func, accel_timestamps, noise.inertial.accel_sigma)

    # Sample features
    print('Sampling features...')
    frame_timestamps = np.linspace(0, options.duration, options.num_frames)
    frames = simulator.generate_feature_observations(scenario.state_func,
                                                     scenario.landmarks,
                                                     scenario.camera,
                                                     frame_timestamps,
                                                     outlier_probability=noise.outlier_probability,
                                                     feature_sigma=noise.feature_sigma)

    return Observables(
        gyro_timestamps=gyro_timestamps,
        gyro_readings=gyro_readings,
        accel_timestamps=accel_timestamps,
        accel_readings=accel_readings,
        frame_timestamps=frame_timestamps,
        frames=frames)


def generate_inertial_observables(options, scenario, noise):
    """
    Generate visual and inertial measurements for a scenario
    """
    # Sample gyro readings
    print('Sampling gyro readings...')
    gyro_timestamps = simulator.generate_timestamps(options.duration, options.gyro_frequency)
    gyro_readings = simulator.generate_gyro_readings(scenario.state_func, gyro_timestamps, noise.inertial.gyro_sigma)

    # Sample accel readings
    print('Sampling accel readings...')
    accel_timestamps = simulator.generate_timestamps(options.duration, options.accel_frequency)
    accel_readings = simulator.generate_accel_readings(scenario.state_func, accel_timestamps, noise.inertial.accel_sigma)

    return InertialObservables(
        gyro_timestamps=gyro_timestamps,
        gyro_readings=gyro_readings,
        accel_timestamps=accel_timestamps,
        accel_readings=accel_readings)
