import unittest
from numericaltesting import assert_arrays_almost_equal
from rigidbody import rotation

from .functions import *


class UnivariatePolynomialTest(unittest.TestCase):
	def test_evaluate(self):
		p = UnivariatePolynomial([1, 2])
		self.assertEqual(p(3.), 7.)
		self.assertEqual(p.derivative()(3.), 2.)
		self.assertEqual(p.derivative()(3.), 2.)
		self.assertEqual(p.derivative(n=2)(3.), 0.)


class PiecewiseFunctionTest(unittest.TestCase):
	def test_evaluate(self):
		p1 = UnivariatePolynomial([1, 2])
		p2 = UnivariatePolynomial([3, 4])
		f = PiecewiseFunction((p1, p2), (1., 3.))
		self.assertEqual(f(0.), p1(-1.))
		self.assertEqual(f(1.), p1(0.))
		self.assertEqual(f(2.), p1(1.))
		self.assertEqual(f(3.), p2(0.))
		self.assertEqual(f(4.), p2(1.))

	def test_derivative(self):
		p1 = UnivariatePolynomial([1, 2])
		p2 = UnivariatePolynomial([3, 4])
		f = PiecewiseFunction((p1, p2), (1., 3.))
		self.assertEqual(f.derivative()(0.), p1.derivative()(-1.))
		self.assertEqual(f.derivative()(1.), p1.derivative()(0.))
		self.assertEqual(f.derivative()(2.), p1.derivative()(1.))
		self.assertEqual(f.derivative()(3.), p2.derivative()(0.))
		self.assertEqual(f.derivative()(4.), p2.derivative()(1.))


class MultidimFunctionTest(unittest.TestCase):
	def test_evaluate(self):
		p1 = UnivariatePolynomial([1, 2])
		p2 = UnivariatePolynomial([3, 4])
		f = MultidimFunction((p1, p2))
		assert_arrays_almost_equal([p1(0.), p2(0.)], f(0.))

	def test_derivative(self):
		p1 = UnivariatePolynomial([1, 2])
		p2 = UnivariatePolynomial([3, 4])
		f = MultidimFunction((p1, p2))
		assert_arrays_almost_equal([p1.derivative()(0.), p2.derivative()(0.)], f.derivative()(0.))


class PoseFunctionTest(unittest.TestCase):
	def test_orientation(self):
		ident = UnivariatePolynomial([0, 1])
		p = MultidimFunction([ident, ident, ident])
		a = MultidimFunction([ident, ident, ident])
		f = PoseFunction(p, a)
		assert_arrays_almost_equal(f.orientation(1), rotation.exp([1, 1, 1]))
		rs = f.orientation([.1, .2, .3, .4])
		self.assertEqual(np.shape(rs), (4, 3, 3))
