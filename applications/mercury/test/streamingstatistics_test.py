import functools
import math
import random
import unittest
from applications.mercury.lib.streamingstatistics import StreamingStatistics


class StreamingStatisticsTest(unittest.TestCase):
    def compute_mean_kahan_summation(self, items):
        # https://en.wikipedia.org/wiki/Kahan_summation_algorithm
        # This is the "best" way I know of trying to do this
        sum = 0
        c = 0
        n = len(items)
        for x in items:
            y = float(x) - c
            t = sum + y
            c = (t - sum) - y
            sum = t

        return sum if n == 0 else sum / float(n)

    def compute_variance_kanan_summation(self, items):
        mean = self.compute_mean_kahan_summation(items)
        transformed_items = [(x - mean) ** 2 for x in items]

        sum = 0
        c = 0
        n = len(items)
        for x in transformed_items:
            y = float(x) - c
            t = sum + y
            c = (t - sum) - y
            sum = t

        return (mean, sum if n <= 1 else sum / (n - 1.0))

    def test_sane_zero_sample_state(self):
        s = StreamingStatistics()
        self.assertAlmostEqual(s.mean(), 0)
        self.assertAlmostEqual(s.variance(), 0)
        self.assertAlmostEqual(s.standard_deviation(), 0)
        self.assertFalse(math.isfinite(s.minimum()))
        self.assertFalse(math.isfinite(s.maximum()))
        self.assertEqual(s.count(), 0)

    def test_sane_single_sample_state(self):
        s = StreamingStatistics()
        s.update(1.0)
        self.assertAlmostEqual(s.mean(), 1.0)
        self.assertAlmostEqual(s.variance(), 0.0)
        self.assertAlmostEqual(s.standard_deviation(), 0.0)
        self.assertEqual(s.minimum(), 1.0)
        self.assertEqual(s.maximum(), 1.0)
        self.assertEqual(s.count(), 1)

    def test_happy_path(self):
        num_samples = 10000
        samples = [random.random() for x in range(num_samples)]

        s = StreamingStatistics()
        for x in samples:
            s.update(x)

        self.assertEqual(s.count(), num_samples)
        (expected_mean, expected_variance) = self.compute_variance_kanan_summation(samples)
        expected_max = functools.reduce(max, samples)
        expected_min = functools.reduce(min, samples)

        self.assertEqual(s.count(), num_samples)
        self.assertAlmostEqual(s.mean(), expected_mean)
        self.assertAlmostEqual(s.variance(), expected_variance)
        self.assertEqual(s.minimum(), expected_min)
        self.assertEqual(s.maximum(), expected_max)
