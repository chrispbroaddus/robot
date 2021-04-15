import unittest
import numpy as np
import rigidbody as rg

from .trajectory import generate_state_function
from ..models import ImuMeasurement
from .imu import *


state_func = generate_state_function(
	"line",
	"forward_facing",
	duration=5.,
	gravity_norm=9.8,
	gyro_bias_sigma=0.,
	accel_bias_sigma=0.,
	initial_gyro_bias_norm=.1,
	initial_accel_bias_norm=.1,
	max_gyro_bias_norm=.2,
	max_accel_bias_norm=.2)

class ImuTest(unittest.TestCase):
	def test_generate_accel_readings(self):
		np.random.seed(0)
		ts = generate_timestamps(3., 20.)
		zs = generate_accel_readings(state_func, ts, 0.)
		self.assertEqual(len(zs), 61)
		self.assertEqual(len(ts), 61)
		self.assertEqual(ts[0], 0.)
		self.assertEqual(ts[-1], 3.)

	def test_generate_gyro_readings(self):
		np.random.seed(0)
		ts = generate_timestamps(3., 20.)
		zs = generate_gyro_readings(state_func, ts, 0.)
		self.assertEqual(len(zs), 61)
		self.assertEqual(len(ts), 61)
		self.assertEqual(ts[0], 0.)
		self.assertEqual(ts[-1], 3.)

	def test_generate_imu_measurements(self):
		np.random.seed(0)
		zs = generate_imu_measurements(state_func, np.linspace(0, 10, 50), 0.)
		self.assertEqual(len(zs), 50)
		self.assertIsInstance(zs[0], ImuMeasurement)

	def test_generate_imu_measurements_between(self):
		np.random.seed(0)
		zs = generate_imu_measurements_between(state_func, 0, 4, 100)
		self.assertEqual(len(zs), 401)
		self.assertIsInstance(zs[0], ImuMeasurement)

	def test_generate_imu_blocks(self):
		np.random.seed(0)
		zs = generate_imu_blocks(state_func, [0, 2, 4], 30)
		self.assertEqual(len(zs), 2)
		self.assertEqual(len(zs[0]), 61)
		self.assertEqual(len(zs[1]), 61)
		self.assertEqual(zs[0][0].timestamp, 0.)
		self.assertEqual(zs[0][-1].timestamp, 2.)
		self.assertEqual(zs[1][0].timestamp, 2.)
		self.assertEqual(zs[1][-1].timestamp, 4.)
