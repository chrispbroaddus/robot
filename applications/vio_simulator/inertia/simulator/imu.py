import numpy as np

from .timestamps import generate_timestamps
from ..models import ImuMeasurement


def generate_gyro_readings(state_func, gyro_timestamps, gyro_sigma, **kwargs):
    """
    Generate gyro readings with noise
    """
    gyro_readings = []
    for t in gyro_timestamps:
        noise = np.random.randn(3) * gyro_sigma
        gyro_readings.append(state_func.gyro(t) + noise)
    return gyro_readings


def generate_accel_readings(state_func, accel_timestamps, accel_sigma, **kwargs):
    """
    Generate acelerometer readings with noise
    """
    accel_readings = []
    for t in accel_timestamps:
        noise = np.random.randn(3) * accel_sigma
        accel_readings.append(state_func.accelerometer(t) + noise)
    return accel_readings


def generate_imu_measurements(state_func, timestamps, gyro_sigma=0., accel_sigma=0.):
    """
    Generate IMU measurements with noise
    """
    measurements = ImuMeasurement.List()
    for t in timestamps:
        gyro_noise = np.random.randn(3) * gyro_sigma
        accel_noise = np.random.randn(3) * accel_sigma
        gyro_reading = state_func.gyro(t) + gyro_noise
        accel_reading = state_func.accelerometer(t) + accel_noise
        measurements.append_new(timestamp=t, gyro_reading=gyro_reading, accel_reading=accel_reading)
    return measurements


def generate_imu_measurements_between(state_func, begin_time, end_time, frequency, **kwargs):
    """
    Generate IMU measurements from begin_time until end_time at the specified frequency (in Hz)
    """
    duration = end_time - begin_time
    timestamps = generate_timestamps(duration, frequency) + begin_time
    return generate_imu_measurements(state_func, timestamps, **kwargs)


def generate_imu_blocks(state_func, frame_timestamps, imu_frequency, **kwargs):
    """
    Generate blocks of IMU measurements between each of the specified timestamps
    """
    blocks = []
    for i in range(len(frame_timestamps) - 1):
        t0, t1 = frame_timestamps[i:i+2]
        blocks.append(generate_imu_measurements_between(state_func, t0, t1, imu_frequency, **kwargs))
    return blocks
