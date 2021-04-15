import unittest
import numpy as np
from numericaltesting import assert_arrays_almost_equal

from .timestamps import *


class TimestampsTest(unittest.TestCase):
	def test_generate_timestamps(self):
		t = generate_timestamps(duration=4, frequency=3)
		self.assertEqual(len(t), 13)
		assert_arrays_almost_equal(t, np.linspace(0, 4, 13))
