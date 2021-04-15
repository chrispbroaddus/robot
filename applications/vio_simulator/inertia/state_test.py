import unittest
import numpy as np
from rigidbody import SE3, rotation
from manifolds import atlas_for
from numericaltesting import assert_arrays_almost_equal

from .state import InertialState

class InertialStateTest(unittest.TestCase):
	def test_constructor(self):
		x = InertialState(
			pose=SE3.from_tangent(np.ones(6)),
			velocity=np.ones(3)*2,
			gyro_bias=np.ones(3)*3,
			accel_bias=np.ones(3)*4)

		assert_arrays_almost_equal(x.pose.position, np.ones(3))
		assert_arrays_almost_equal(x.pose.orientation, rotation.exp([1, 1, 1]))
		assert_arrays_almost_equal(x.velocity, np.ones(3)*2)
		assert_arrays_almost_equal(x.gyro_bias, np.ones(3)*3)
		assert_arrays_almost_equal(x.accel_bias, np.ones(3)*4)

	def test_from_array(self):
		x = InertialState.from_array([1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 3, 3, 2, 2, 2, 4, 4, 4, 0, 0, 0])
		assert_arrays_almost_equal(x.pose.position, np.zeros(3))
		assert_arrays_almost_equal(x.pose.orientation, np.eye(3))
		assert_arrays_almost_equal(x.velocity, np.ones(3)*2)
		assert_arrays_almost_equal(x.gyro_bias, np.ones(3)*3)
		assert_arrays_almost_equal(x.accel_bias, np.ones(3)*4)

	def test_from_tangent(self):
		x = InertialState.from_tangent([0, 0, 0, 3, 3, 3, 2, 2, 2, 4, 4, 4, 0, 0, 0])
		assert_arrays_almost_equal(x.pose.position, np.zeros(3))
		assert_arrays_almost_equal(x.pose.orientation, np.eye(3))
		assert_arrays_almost_equal(x.velocity, np.ones(3)*2)
		assert_arrays_almost_equal(x.gyro_bias, np.ones(3)*3)
		assert_arrays_almost_equal(x.accel_bias, np.ones(3)*4)

	def test_flatten(self):
		x = InertialState(
			pose=SE3.from_tangent(np.zeros(6)),
			velocity=np.ones(3)*2,
			gyro_bias=np.ones(3)*3,
			accel_bias=np.ones(3)*4)

		expected = [1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 3, 3, 2, 2, 2, 4, 4, 4, 0, 0, 0]
		assert_arrays_almost_equal(x.flatten(), expected)

	def test_origin(self):
		x = InertialState.origin()
		assert_arrays_almost_equal(x.pose.position, np.zeros(3))
		assert_arrays_almost_equal(x.pose.orientation, np.eye(3))
		assert_arrays_almost_equal(x.velocity, np.zeros(3))
		assert_arrays_almost_equal(x.gyro_bias, np.zeros(3))
		assert_arrays_almost_equal(x.accel_bias, np.zeros(3))

	def test_position_property(self):
		x = InertialState.origin()
		assert_arrays_almost_equal(x.position, np.zeros(3))

	def test_orientation_property(self):
		x = InertialState.origin()
		assert_arrays_almost_equal(x.orientation, np.eye(3))

	def test_position_setter(self):
		x = InertialState.origin()
		x.position = np.ones(3)
		assert_arrays_almost_equal(x.position, np.ones(3))

	def test_orientation_setter(self):
		x = InertialState.origin()
		x.orientation = rotation.exp([1, 2, 3])
		assert_arrays_almost_equal(x.orientation, rotation.exp([1, 2, 3]))

	def test_atlas(self):
		x = InertialState.origin()
		self.assertIs(atlas_for(x), InertialState.Atlas)

	def test_dof(self):
		x = InertialState.origin()
		a = atlas_for(x)
		self.assertEqual(a.dof(x), 15)

	def test_perturb(self):
		x = InertialState.origin()
		a = atlas_for(x)
		y = a.perturb(x, np.ones(15))
		assert_arrays_almost_equal(y.gyro_bias, np.ones(3))
		assert_arrays_almost_equal(y.position, np.ones(3))
