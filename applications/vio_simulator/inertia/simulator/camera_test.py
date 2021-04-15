import unittest
import numpy as np
from numericaltesting import assert_arrays_almost_equal
from rigidbody import pr, SE3

from .trajectory import generate_state_function
from .camera import *

state_func = generate_state_function(
	"line",
	"forward_facing",
	duration=5.,
	gravity_norm=9.8,
	gyro_bias_sigma=0.,
	accel_bias_sigma=0.)

class CameraTest(unittest.TestCase):
	def test_generate_landmarks(self):
		np.random.seed(0)
		landmarks = generate_landmarks(state_func, 2., "cloud", landmarks_per_second=100)
		self.assertEqual(len(landmarks), 200)

	def test_generate_camera_matrix(self):
		np.random.seed(0)
		c = generate_camera_matrix(640, 480)
		expected = [
			[320, 0, 320],
			[0, 240, 240],
			[0, 0, 1]
		]

		assert_arrays_almost_equal(c, expected)

	def test_generate_feature_observations(self):
		np.random.seed(0)
		landmarks = np.random.randn(100, 4) * 100
		t = SE3.from_tangent(np.random.randn(6))

		camera = generate_camera(image_xlim=(-5, 5), image_ylim=(-5, 5))

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

		frames = generate_feature_observations(
			state_func,
			landmarks,
			camera=camera,
			frame_timestamps=np.linspace(0, 5, 4),
			device_to_camera_transform=t,
			feature_sigma=1e-3)

		self.assertEqual(len(frames), 4)
		self.assertIsInstance(frames[0], Frame)

		feature = frames[0].true_observations[0]

		r0, p0 = state_func(0.).pose.rp
		rc, pc = t.rp

		x0 = landmarks[frames[0].landmark_ids[0]]
		y0_device = np.dot(r0, x0[:3] - p0*x0[3])
		y0_camera = np.dot(rc, y0_device[:3] - pc*x0[3])

		z0 = pr(np.dot(camera.matrix, y0_camera))

		assert_arrays_almost_equal(z0, feature)
