#pragma once

#undef DEBUG_CONVEX_HULL

#include "Eigen/Dense"
#include "glog/logging.h"

namespace geometry {

///
/// \return True if the path is counter-clockwise.
/// \details Method : applying the cross multiplication and check the sign.
///
template <typename T>
bool isCounterClockwisePath(
    const Eigen::Matrix<T, 2, 1>& prevPoint, const Eigen::Matrix<T, 2, 1>& currentPoint, const Eigen::Matrix<T, 2, 1>& nextPoint) {
    return (currentPoint(0) - prevPoint(0)) * (nextPoint(1) - prevPoint(1))
        - (currentPoint(1) - prevPoint(1)) * (nextPoint(0) - prevPoint(0))
        > 0;
}

#ifdef DEBUG_CONVEX_HULL
template <typename T> bool printNumbers(const std::vector<Eigen::Matrix<T, 2, 1> >& points, int mp, int m, int i) {
    LOG(INFO) << " m-1 : " << points[mp](0) << "," << points[mp](1) << ", m : " << points[m](0) << "," << points[m](1)
              << ", i : " << points[i](0) << "," << points[i](1);
    return true;
}
#endif

///
/// \return the integer `m`, where the first `m` points forming a convex hull of the input points.
/// \details https://en.wikipedia.org/wiki/Graham_scan
///
template <typename T> int calculateConvexHullDestructive(std::vector<Eigen::Matrix<T, 2, 1> >& points) {

    if (points.size() <= 2) {
        LOG(WARNING) << "The minimum required number of points to calculate a convex hull is 3.";
        return 0;
    }

    // Swap the points to let points[0] to have a lowest y-coordinate value.
    for (size_t i = 1; i < points.size(); i++) {
        if (points[i](1) < points[0](1)) {
            std::swap(points[i], points[0]);
        } else if (points[i](1) == points[0](1)) {
            // handling edge case when there exists multiple lowest y-coordinated points
            if (points[i](0) == points[0](0)) {
                std::swap(points[i], points[0]);
            }
        }
    }

    // Sort points[1:end] in angle by ascending order of theta (=descending order of cos in [0,pi]).
    const auto refPoint = points[0];
    std::sort(points.begin() + 1, points.end(), [refPoint](const Eigen::Matrix<T, 2, 1>& pointA, const Eigen::Matrix<T, 2, 1>& pointB) {
        if (pointA == refPoint || pointB == refPoint) {
            return true;
        }
        T ca = (pointA(0) - refPoint(0)) / (pointA - refPoint).norm();
        T cb = (pointB(0) - refPoint(0)) / (pointB - refPoint).norm();
        return cb < ca;
    });

    int m = 1;
    for (size_t i = 2; i < points.size(); i++) {

        // Find next valid point on convex hull
        while (
#ifdef DEBUG_CONVEX_HULL
            printNumbers(points, m - 1, m, i) &&
#endif
            isCounterClockwisePath(points[m - 1], points[m], points[i]) <= 0) {
            if (m > 1) {
                m -= 1;
                continue;
            } else if (i == points.size()) {
                break;
            } else {
                i += 1;
            }
        }

        // Update `m` and swap points[i] to the correct place.
        m += 1;
        std::swap(points[m], points[i]);
    }
    return m + 1;
}
}