import bisect
import numpy as np
from scipy.interpolate import InterpolatedUnivariateSpline
from rigidbody import rotation

from .functions import UnivariatePolynomial, PiecewiseFunction, MultidimFunction


def make_ramp_up(dt, z0, z1):
    """
    Create a quadratic such that f(0)=z0, f(dt)=z1, and f'(0)=0.
    """
    assert dt > 0
    return UnivariatePolynomial([z0, 0., (z1-z0)/(dt*dt)])


def make_ramp_down(dt, z0, z1):
    """
    Create a quadratic such that f(0)=z0, f(dt)=z1, and f'(dt)=0.
    """
    assert dt > 0
    return UnivariatePolynomial([z0, 2*(z1-z0)/dt, (z0-z1)/(dt*dt)])


def make_piecewise_quadratic(ts, zs):
    """
    Create a function such that f(t_i) = y_i and f'(t_i) = 0 for i = 1...N
    """
    assert len(ts) == len(zs)
    curves = []
    begin_times = []
    for i in range(len(zs) - 1):
        t0, t1 = ts[i], ts[i+1]
        z0, z1 = zs[i], zs[i+1]
        dt = t1 - t0
        tmid = (t0 + t1) / 2.
        ymid = (z0 + z1) / 2.
        curves.append(make_ramp_up(dt/2., z0, ymid))
        begin_times.append(t0)
        curves.append(make_ramp_down(dt/2., ymid, z1))
        begin_times.append(tmid)
    return PiecewiseFunction(curves, begin_times)


def interpolate_spline(ts, xs, degree=3):
    """
    Fit a spline through a sequences of points.
    """
    xs = np.asarray(xs)
    assert np.all(np.diff(ts) > 0), 'timestamps should be monotonic'
    assert len(ts) == len(xs)
    assert len(xs) >= degree + 1, 'need %d points to fit a spline of degree %d but got %d' % (degree+1, degree, len(xs))
    assert xs.ndim == 2, 'shape was %s' % str(xs.shape)
    return MultidimFunction([InterpolatedUnivariateSpline(ts, col, k=degree) for col in xs.T])


def interpolate_orientation_spline(ts, orientations):
    """
    Fit a spline through a sequence of orientations.
    """
    axisangles = np.array(list(map(rotation.log, orientations)))
    # The log map wraps around at ||axisangle||=2pi. But this wrap-around causes all sorts of problems when we fit a
    # spline through our axisangle parameters. The real solution here is to use a cumulative B-spline as in the Spline
    # Fusion paper. But for now we have this hack...
    for i in range(1, len(axisangles)):
        axisangles[i] = rotation.unroll_axisangle(axisangles[i], axisangles[i-1])
    return interpolate_spline(ts, axisangles)


def interpolate_piecewise_straight(ts, points):
    """
    Create a parametric curve that moves in a straight line between each vertex in the 
    specified list. Note however that the generated motion is _not_ linear: rather, the 
    device moves smoothly between each of the points. In particular the motion is 
    everywhere twice differentiable and the acceleration curve is everywhere continuous.
    """
    points = np.asarray(points)
    return MultidimFunction([make_piecewise_quadratic(ts, col) for col in points.T])
