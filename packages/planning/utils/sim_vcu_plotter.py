#!/usr/bin/env python

import argparse
import os

from dateutil import parser
from datetime import  datetime
from matplotlib import pyplot
import numpy

def plot(logfile):
    assert os.path.isfile(logfile)
  
    VELOCITY_MARKER = 'velocity'
    T_V = []
    V = []
    STEER_MARKER = 'curvature'
    T_C = []
    C = []

    with open(logfile, 'r') as handle:
        for line in handle:
            if VELOCITY_MARKER in line:
                components = line.strip().split()
                time = components[1]
                velocity = float(components[8])
                if velocity == 0.0:
                    continue
                V.append(velocity)
                T_V.append((parser.parse(time) - datetime(1970,1,1)).total_seconds())
            if STEER_MARKER in line:
                components = line.strip().split()
                time = components[1]
                steer = float(components[7])
                if steer == 0.0:
                    continue
                T_C.append((parser.parse(time) - datetime(1970,1,1)).total_seconds())
                C.append(steer)

    T_V = numpy.array(T_V)
    T_V = T_V - T_V[0]
 
    T_C = numpy.array(T_C)
    T_C = T_C - T_C[0]
    
    pyplot.figure()
    pyplot.plot(T_V, V, 'b')
    pyplot.plot(T_C, C, 'r')
    pyplot.show()

def main():
    parser = argparse.ArgumentParser()
    LOG = os.path.expanduser('~/.config/unity3d/Zippy/ZippySim/Player.log')
    plot(LOG)

if __name__ == "__main__":
    main()
