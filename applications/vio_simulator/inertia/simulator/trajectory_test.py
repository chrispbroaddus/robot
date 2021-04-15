import unittest
import numpy as np
from numericaltesting import assert_arrays_almost_equal

from .trajectory import *


class TrajectoryTest(unittest.TestCase):
	def test_generate_state_function_line(self):
		np.random.seed(0)
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

		assert_arrays_almost_equal(np.zeros(3), state_func(0.).position)
		self.assertGreater(np.linalg.norm(state_func(0.).velocity), 0.)
		self.assertEqual(state_func(0.).orientation.shape, (3, 3))
		assert_arrays_almost_equal(np.linalg.norm(state_func(0.).gyro_bias), .1)
		assert_arrays_almost_equal(np.linalg.norm(state_func(0.).accel_bias), .1)

	def test_generate_state_function_manhattan_2d(self):
		np.random.seed(0)
		state_func = generate_state_function(
			"manhattan_2d",
			"forward_facing",
			duration=5.,
			gravity_norm=9.8,
			gyro_bias_sigma=0.,
			accel_bias_sigma=0.,
			initial_gyro_bias_norm=.1,
			initial_accel_bias_norm=.1,
			max_gyro_bias_norm=.2,
			max_accel_bias_norm=.2)

		assert_arrays_almost_equal(np.zeros(3), state_func(0.).position)
		self.assertGreater(np.linalg.norm(state_func(0.).velocity), 0.)
		self.assertEqual(state_func(0.).orientation.shape, (3, 3))
		assert_arrays_almost_equal(np.linalg.norm(state_func(0.).gyro_bias), .1)
		assert_arrays_almost_equal(np.linalg.norm(state_func(0.).accel_bias), .1)

	def test_generate_state_function_circle_2d(self):
		np.random.seed(0)
		state_func = generate_state_function(
			"circle_2d",
			"forward_facing",
			duration=5.,
			gravity_norm=9.8,
			gyro_bias_sigma=0.,
			accel_bias_sigma=0.,
			initial_gyro_bias_norm=.1,
			initial_accel_bias_norm=.1,
			max_gyro_bias_norm=.2,
			max_accel_bias_norm=.2)

		assert_arrays_almost_equal([0, 1, 0], state_func(0.).position)
		self.assertGreater(np.linalg.norm(state_func(0.).velocity), 0.)
		self.assertEqual(state_func(0.).orientation.shape, (3, 3))
		assert_arrays_almost_equal(np.linalg.norm(state_func(0.).gyro_bias), .1)
		assert_arrays_almost_equal(np.linalg.norm(state_func(0.).accel_bias), .1)

	def test_generate_state_function_square_2d(self):
		np.random.seed(0)
		state_func = generate_state_function(
			"square_2d",
			"forward_facing",
			duration=5.,
			gravity_norm=9.8,
			gyro_bias_sigma=0.,
			accel_bias_sigma=0.,
			initial_gyro_bias_norm=.1,
			initial_accel_bias_norm=.1,
			max_gyro_bias_norm=.2,
			max_accel_bias_norm=.2)

		assert_arrays_almost_equal(np.zeros(3), state_func(0.).position)
		self.assertEqual(np.linalg.norm(state_func(0.).velocity), 0.)
		self.assertEqual(state_func(0.).orientation.shape, (3, 3))
		assert_arrays_almost_equal(np.linalg.norm(state_func(0.).gyro_bias), .1)
		assert_arrays_almost_equal(np.linalg.norm(state_func(0.).accel_bias), .1)

	def test_generate_state_function_random_walk(self):
		np.random.seed(0)
		state_func = generate_state_function(
			"random_walk",
			"forward_facing",
			duration=5.,
			gravity_norm=9.8,
			gyro_bias_sigma=0.,
			accel_bias_sigma=0.,
			initial_gyro_bias_norm=.1,
			initial_accel_bias_norm=.1,
			max_gyro_bias_norm=.2,
			max_accel_bias_norm=.2)

		assert_arrays_almost_equal(np.zeros(3), state_func(0.).position)
		self.assertGreater(np.linalg.norm(state_func(0.).velocity), 0.)
		self.assertEqual(state_func(0.).orientation.shape, (3, 3))
		assert_arrays_almost_equal(np.linalg.norm(state_func(0.).gyro_bias), .1)
		assert_arrays_almost_equal(np.linalg.norm(state_func(0.).accel_bias), .1)

	def test_generate_state_function_random_walk_2d(self):
		np.random.seed(0)
		state_func = generate_state_function(
			"random_walk_2d",
			"forward_facing",
			duration=5.,
			gravity_norm=9.8,
			gyro_bias_sigma=0.,
			accel_bias_sigma=0.,
			initial_gyro_bias_norm=.1,
			initial_accel_bias_norm=.1,
			max_gyro_bias_norm=.2,
			max_accel_bias_norm=.2)

		assert_arrays_almost_equal(np.zeros(3), state_func(0.).position)
		self.assertGreater(np.linalg.norm(state_func(0.).velocity), 0.)
		self.assertEqual(state_func(0.).orientation.shape, (3, 3))
		assert_arrays_almost_equal(np.linalg.norm(state_func(0.).gyro_bias), .1)
		assert_arrays_almost_equal(np.linalg.norm(state_func(0.).accel_bias), .1)
