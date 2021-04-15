import argparse
import struct

from packages.hal.proto.camera_sample_pb2 import *
from packages.perception.utils.utils import read_frames

def trim(point_cloud_file, frame, output=None):
    images = read_frames(point_cloud_file)
    target = images[frame]
  
    if not output:
        output = 'trimmed.pb'

    with open(output, 'wb') as handle:
        payload = target.SerializeToString()
        handle.write(struct.pack('<I', len(payload)))
        handle.write(payload)

def main():
    parser = argparse.ArgumentParser(description='Point cloud plotter')
    parser.add_argument('point_cloud_file')
    parser.add_argument('frame', type=int)
    parser.add_argument('--output')
    trim(**vars(parser.parse_args()))

if __name__ == "__main__":
    main()
