import array
import io
import os
import struct

import numpy as np

from packages.hal.proto.camera_sample_pb2 import *

def image_to_points(frame, downsample=20):
    floats = frame.image.rows * frame.image.cols *3
    buffer = io.BytesIO(frame.image.data)
    points = []
    data = array.array('f')
    data.fromfile(buffer, floats)
    points = np.array(data)
    shape = points.shape
    points = points.reshape(int(shape[0]/3.0), 3).transpose()
    return points[:,::downsample]

def read_frames(point_cloud_file):
    assert os.path.isfile(point_cloud_file)
    samples = []
    with open(point_cloud_file, 'rb') as handle:
        while (True):
            proto_size_raw = handle.read(4)
            if len(proto_size_raw) == 0:
                break
            proto_size = struct.unpack('<i', proto_size_raw)
            proto = handle.read(proto_size[0])
            sample = CameraSample()
            sample.ParseFromString(proto)
            samples.append(sample)
    return samples
