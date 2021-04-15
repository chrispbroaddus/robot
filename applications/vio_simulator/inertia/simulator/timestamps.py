import numpy as np


def generate_timestamps(duration, frequency):
	"""
	Generate timestamps between 0 and duration at the given frequency.
	If duration * frequency is an integer then there will be a timestamp
	at the end point.
	"""
	n = duration * frequency
	if abs(n - round(n)) < 1e-8:
		return np.linspace(0, duration, n+1)
	else:
		return np.arange(0, duration, 1./frequency)
