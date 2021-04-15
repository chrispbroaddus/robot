import unittest
import numpy as np

from .interpolate import *
from rigidbody import rotation
from numericaltesting import assert_arrays_almost_equal


class InterpolateTest(unittest.TestCase):
	def test_interpolate_spline(self):
		ts = np.arange(5)
		xs = np.arange(15).reshape((5, 3))
		f = interpolate_spline(ts, xs)
		assert_arrays_almost_equal(f(0), xs[0])
		assert_arrays_almost_equal(f(1), xs[1])
		assert_arrays_almost_equal(f(4), xs[4])

	def test_interpolate_orientations(self):
		r0 = rotation.exp([0, 0, 0])
		r1 = rotation.exp([1, 0, 0])
		r2 = rotation.exp([0, 1, 0])
		r3 = rotation.exp([0, 0, 1])
		f = interpolate_orientation_spline([0, 1, 2, 3], [r0, r1, r2, r3])
		assert_arrays_almost_equal(f(0), rotation.log(r0))
		assert_arrays_almost_equal(f(1), rotation.log(r1))
		assert_arrays_almost_equal(f(2), rotation.log(r2))
		assert_arrays_almost_equal(f(3), rotation.log(r3))

	def test_interpolate_orientations_unroll(self):
		logrs = [[0, 0, i] for i in range(5)]
		f = interpolate_orientation_spline(list(range(5)), list(map(rotation.exp, logrs)))
		assert_arrays_almost_equal(f(0), logrs[0])
		assert_arrays_almost_equal(f(1), logrs[1])
		assert_arrays_almost_equal(f(2), logrs[2])
		assert_arrays_almost_equal(f(3), logrs[3])
		assert_arrays_almost_equal(f(4), logrs[4])
