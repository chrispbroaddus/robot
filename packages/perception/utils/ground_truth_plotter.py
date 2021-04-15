import argparse
import os
import struct

from matplotlib import pyplot
import numpy

from packages.unity_plugins.proto import unity_telemetry_envelope_pb2
from packages.unity_plugins.proto import ground_truth_vehicle_pose_pb2 

def read(filename):
    assert os.path.isfile(filename)
    protos = []
    with open(filename, 'rb') as handle:
        while True:
            bytes_to_read = handle.read(4)
            if (not bytes_to_read):
                break
            proto_size = struct.unpack('<I', bytes_to_read)[0]
            T = unity_telemetry_envelope_pb2.UnityTelemetryEnvelope(); 
            T.MergeFromString(handle.read(proto_size))
            protos.append(T)
    return protos

def plot(filename):
    protos = read(filename)
    T = []

    for entry in protos:
        T.append((entry.vehiclePose.measurementSystemTimestamp.nanos,
                  entry.vehiclePose.transformations.translationX,
                  entry.vehiclePose.transformations.translationY,
                  entry.vehiclePose.transformations.translationZ))

    T = numpy.array(T)
        
    f, ax = pyplot.subplots(3)
    ax[0].plot((T[:,0] - T[0,0])/1e9, T[:,1])
    ax[1].plot((T[:,0] - T[0,0])/1e9, T[:,2])
    ax[2].plot((T[:,0] - T[0,0])/1e9, T[:,3])
    pyplot.savefig('output.png')
    pyplot.show()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('filename')
    plot(**vars(parser.parse_args()))

if __name__ == "__main__":
    main()
