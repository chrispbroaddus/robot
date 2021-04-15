import math
import sys
import time

import matplotlib.pyplot as plt
import numpy as np
import visvis as vv


def load_file(file_name):
    print('Loading model from [{}]'.format(file_name))

    with open(file_name, "rt") as input:
        line = input.readline()
        volume_length = float(line)

        line = input.readline()
        count = int(line)

        voxel_centers = np.zeros((count, 3), dtype=np.float32)
        content_counts = np.zeros((count, 1), dtype=np.float32)
        centroids = np.zeros((count, 3), dtype=np.float32)

        for r in range(count):
            data = input.readline().split('\t')
            # address = int(data[0])
            voxelCenter = (float(data[1]), float(data[2]), float(data[3]))
            centroid = (float(data[4]), float(data[5]), float(data[6]))
            count = int(data[7])

            voxel_centers[r, :] = voxelCenter
            content_counts[r] = count
            centroids[r, :] = centroid

        return (volume_length, voxel_centers, centroid, content_counts)


def histogram_occupancies(content_counts):
    print('Plotting histogram of occupancy counts')

    f = plt.figure(figsize=(8, 11), dpi=300)
    (histogram, bin_edges) = np.histogram(content_counts, 'fd', density=True)

    bin_centers = []
    for x in range(1, len(bin_edges)):
        bin_centers.append((bin_edges[x - 1] + bin_edges[x]) / 2)

    start = time.time()

    plt.bar(bin_centers, histogram)
    plt.title('Distribution of voxel occupancy counts')
    plt.xlabel('# of points')
    plt.ylabel('% of voxels')
    plt.legend()
    f.savefig('occupancy-histogram.pdf', orientation='landscape', papertype='portrait')
    plt.close(f)

    elapsed = time.time() - start
    print('Plotting required [{}] seconds'.format(elapsed))

    print('Occupancy counts in occupancy-histogram.pdf')


def plot_voxel_centers(volume_length, voxel_centers):
    vv.figure()

    a = vv.gca()

    point_color = 'r'
    point_size = '4'
    points_alpha = 0.1
    points = vv.Pointset(voxel_centers)

    objects = vv.plot(points, ms='.', mc=point_color, mw=point_size, ls='', mew=0)
    objects.alpha = points_alpha

    # Set settings for axes
    a.axis.xLabel = 'X'
    a.axis.yLabel = 'Y'
    a.axis.zLabel = 'Z'

    # Set black bg
    a.bgcolor = 'k'
    a.axis.axisColor = 'w'
    a.axis.showGrid = True

    # Enter mainloop
    app = vv.use()
    app.Run()


def main():
    volume_length, voxel_centers, centroids, content_counts = load_file(sys.argv[1])

    histogram_occupancies(content_counts)
    plot_voxel_centers(volume_length, voxel_centers)


if "__main__" == __name__:
    main()
