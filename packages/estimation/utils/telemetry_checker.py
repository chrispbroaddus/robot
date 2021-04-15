import argparse
import os
import struct

import numpy
from matplotlib import pyplot

from packages.hal.proto import vcu_telemetry_envelope_pb2


def _read_one(stream):
    """Read a byte from the file (as an integer)
    raises EOFError if the stream ends while reading bytes.
    """
    c = stream.read(1)
    if c == '':
        raise EOFError("Unexpected EOF while reading bytes")
    return ord(c)

def decode_stream(stream):
    """Read a varint from `stream`"""
    shift = 0
    result = 0
    while True:
        i = _read_one(stream)
        result |= (i & 0x7f) << shift
        shift += 7
        if not (i & 0x80):
            break
    return result

def analyze(telemetry):
    assert os.path.isfile(telemetry)
    assert telemetry.endswith('.protodat')

    wheel = []
    steer = []

    with open(telemetry, 'rb') as handle:
        while (True):
            try:
                payloadSize = decode_stream(handle)
            except:
                break
            payload = handle.read(payloadSize)
            msg = vcu_telemetry_envelope_pb2.VCUTelemetryEnvelope().FromString(payload)
            if msg.HasField("servo"):
                steer.append(msg)
            elif msg.HasField("wheelEncoder"):
                wheel.append(msg)
    
    D = []
    V = []
    T = []
    DT = []
    for msg in wheel:
        dt = msg.wheelEncoder.periodEndHardwareTimestamp.nanos - \
             msg.wheelEncoder.periodStartHardwareTimestamp.nanos
        dt = dt / 1e9
        DT.append(dt)
        travel = msg.wheelEncoder.linearDisplacementMeters
        T.append(msg.wheelEncoder.periodStartHardwareTimestamp.nanos / 1e9)
        V.append(travel/dt)
        D.append(travel)

    T = numpy.array(T)
    T = T - T[0]

    f, (ax1, ax2, ax3) = pyplot.subplots(3, 1, sharex=True)
    ax1.set_title(os.path.basename(telemetry))
    ax1.plot(T, D, 'r')
    ax1.set_ylabel('D (m)')
    ax2.plot(T, DT, 'g')
    ax2.set_ylabel('T (s)')
    ax3.plot(T, V, 'b')
    ax3.set_ylabel('V (m/s)')
    pyplot.savefig('{}.png'.format(os.path.basename(telemetry)))
    pyplot.show()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('telemetry')
    analyze(**vars(parser.parse_args()))

if __name__ == "__main__":
    main()
