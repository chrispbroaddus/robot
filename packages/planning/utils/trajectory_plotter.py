import argparse
import os
import sys
import struct

from matplotlib import pyplot
import numpy

from packages.planning.proto import trajectory_pb2 as trajectory

def read(filename):
    assert os.path.isfile(filename)
    protos = []
    
    with open(filename, 'rb') as handle:
        while True:
            bytes_to_read = handle.read(4)
            if (not bytes_to_read):
                break
            proto_size = struct.unpack('<I', bytes_to_read)[0]
            T = trajectory.Trajectory()
            T.MergeFromString(handle.read(proto_size))
            protos.append(T)
    return protos

def plot(filename):
    protos = read(filename)
    sys.stdout.write('{} protos\n'.format(len(protos)))     
    
    T0 = None

    segments = []
    for entry in protos:
        segment = []
        for element in entry.elements:
            T = element.absolute_time.nanos/1e9
            if T0 is None:
                T0 = T
            V = element.linear_velocity
            C = element.curvature
            segment.append((T,V,C))
        segments.append(segment)
    
    f, ax = pyplot.subplots(2, sharex=True)
    for segment in segments:
        for element in segment:
            T, V, C = element
            T = T - T0
            print (T, V)
            ax[0].hold(True)
            ax[0].plot(T, V)
            ax[1].plot(T, C, 'r')
    pyplot.show()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('filename')
    plot(**vars(parser.parse_args()))

if __name__ == "__main__":
    main()
