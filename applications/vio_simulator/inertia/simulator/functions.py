import bisect
import numpy as np
from scipy.interpolate import InterpolatedUnivariateSpline
from rigidbody import rotation, SE3
from ..state import InertialState


class UnivariatePolynomial(object):
    """
    Represents a polynomial in one variable.
    """
    def __init__(self, coefficients):
        """
        Initialize this polynomial with a coefficient vector
        """
        self._coefficients = coefficients

    def __call__(self, x):
        """
        Evaluate the polynomial at x
        """
        return sum(c * x ** i for i, c in enumerate(self._coefficients))

    def _first_derivative(self):
        """
        Get a UnivariatePolynomial representing the first derivative of this polynomial
        """
        return UnivariatePolynomial([c*i for i, c in enumerate(self._coefficients[1:], start=1)])

    def derivative(self, n=1):
        """
        Get a UnivariatePolynomial representing the n-th derivative of this polynomial
        """
        out = self
        for _ in range(n):
            out = out._first_derivative()
        return out


class PiecewiseFunction(object):
    """
    Represents a function constructed by concatenating one or more functions.
    """
    def __init__(self, curves, begin_times):
        assert len(curves) == len(begin_times)
        self._curves = curves
        self._begin_times = begin_times

    def __call__(self, x):
        """
        Evaluate this function at x.
        """
        # call to max() below will cause us to extrapolate the first segment backwards, which is fine
        try:
            # first attempt to treat x as a list
            return np.array([self(t) for t in x])
        except TypeError:
            # x is not iterable so treat it as a scalar
            index = max(0, bisect.bisect(self._begin_times, x) - 1)
            return self._curves[index](x - self._begin_times[index])

    def derivative(self, n=1):
        """
        Get a PiecewiseFunction representing the n-th derivative of this function
        """
        return PiecewiseFunction([curve.derivative(n) for curve in self._curves], self._begin_times)


class MultidimFunction(object):
    """
    Represents a parametric curve though N dimensional space. Each dimension
    is represented as an independent function of time.
    """
    def __init__(self, fs):
        self._fs = fs

    def __call__(self, t):
        """
        Evaluate the curve at time t.
        """
        return np.array([f(t) for f in self._fs]).T

    def derivative(self, n=1):
        """
        Get a MultidimFunction representing the n-th derivative of this curve
        """
        return MultidimFunction([f.derivative(n) for f in self._fs])


class PoseFunction(object):
    """
    Represents a position and orientation of a device at every point in time from t0 to t1.
    """

    def __init__(self, position_curve, axisangle_curve):
        self.position = position_curve
        self.velocity = position_curve.derivative(1)
        self.acceleration = position_curve.derivative(2)
        self.axisangle = axisangle_curve
        self.axisangle_rate = axisangle_curve.derivative(1)

    @classmethod
    def fit_spline(cls, timestamps, positions, orientations, degree=3):
        """
        Fit a spline through the given positions and orientations
        """
        assert len(positions) == len(orientations)
        return PoseFunction(curve.fit_spline(timestamps, positions, degree=degree),
                            curve.fit_orientation_curve(timestamps, orientations))

    def orientation(self, t):
        """
        Get a 3x3 matrix representing the orientation at time t.
        """
        if np.isscalar(t):
            return rotation.exp(self.axisangle(t))
        else:
            return list(map(rotation.exp, self.axisangle(np.asarray(t))))

    def __call__(self, t):
        """
        Get the pose of the device at time t.
        """
        r = self.orientation(t)
        p = self.position(t)
        return SE3(self.orientation(t), self.position(t))

    def angular_velocity_global(self, t):
        """
        Get the angular velocity at time t in the world frame.
        """
        return rotation.angular_velocity_from_axisangle_rates(self.axisangle(t), self.axisangle_rate(t))

    def angular_velocity_local(self, t):
        """
        Get the angular velocity at time t in the local frame.
        """
        return np.dot(self.orientation(t), self.angular_velocity_global(t))


class StateFunction(object):
    """
    Represents a pose, accel bias, and gyro bias for a device at every point in time from t0 to t1.
    """

    def __init__(self, trajectory, gyro_bias_curve, accel_bias_curve, gravity):
        """
        Initialize with a trajectory function, and two bias functions.
        """
        assert np.shape(gravity) == (3,)
        assert isinstance(trajectory, PoseFunction)
        self.gravity = np.asarray(gravity)
        self.trajectory = trajectory
        self.gyro_bias = gyro_bias_curve
        self.accel_bias = accel_bias_curve

    def accelerometer(self, t):
        """
        Get the predicted accelerometer reading at time t.
        """
        r = self.trajectory.orientation(t)
        return np.dot(r, self.trajectory.acceleration(t)) + np.dot(r, self.gravity) + self.accel_bias(t)

    def gyro(self, t):
        """
        Get the predicted gyro reading at time t.
        """
        return self.trajectory.angular_velocity_local(t) + self.gyro_bias(t)

    def __call__(self, t):
        """
        Get the state at time t.
        """
        return InertialState(
            pose=SE3(self.trajectory.orientation(t), self.trajectory.position(t)),
            velocity=self.trajectory.velocity(t),
            gyro_bias=self.gyro_bias(t),
            accel_bias=self.accel_bias(t))
