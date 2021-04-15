import argparse

from packages.hal.proto.camera_sample_pb2 import *

from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt

from packages.perception.utils.utils import read_frames, image_to_points

def render(point_cloud_file):
    images = read_frames(point_cloud_file)
    clouds = [image_to_points(frame) for frame in images]
    assert len(clouds) > 0
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    plt.ion()
    for cloud in clouds:
        ax.scatter(cloud[0,:], cloud[1,:], cloud[2,:], s=0.1)
        ax.set_xlim([-10, 10])
        ax.set_ylim([-10, 10])
        ax.set_zlim([-10, 10])
        plt.pause(0.1)
        ax.clear()
    plt.show(block=True)

def main():
    parser = argparse.ArgumentParser(description='Point cloud plotter')
    parser.add_argument('point_cloud_file')
    render(**vars(parser.parse_args()))

if __name__ == "__main__":
    main()
