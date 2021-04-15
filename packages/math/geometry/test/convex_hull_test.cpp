#include "packages/math/geometry/convex_hull.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

using namespace geometry;

TEST(CalculateConvexHullDestructive, checkCorrectnessExample1) {
    std::vector<Eigen::Matrix<float, 2, 1> > points;

    {
        Eigen::Matrix<float, 2, 1> point(0, 3);
        points.push_back(point);
    }
    {
        Eigen::Matrix<float, 2, 1> point(1, 1);
        points.push_back(point);
    }
    {
        Eigen::Matrix<float, 2, 1> point(2, 2);
        points.push_back(point);
    }
    {
        Eigen::Matrix<float, 2, 1> point(4, 4);
        points.push_back(point);
    }
    {
        Eigen::Matrix<float, 2, 1> point(0, 0);
        points.push_back(point);
    }
    {
        Eigen::Matrix<float, 2, 1> point(1, 2);
        points.push_back(point);
    }
    {
        Eigen::Matrix<float, 2, 1> point(3, 1);
        points.push_back(point);
    }
    {
        Eigen::Matrix<float, 2, 1> point(3, 3);
        points.push_back(point);
    }

    int numConvexHullPoints = calculateConvexHullDestructive(points);
    EXPECT_EQ(numConvexHullPoints, 4);

    EXPECT_EQ(points[0](0), 0);
    EXPECT_EQ(points[0](1), 0);

    EXPECT_EQ(points[1](0), 3);
    EXPECT_EQ(points[1](1), 1);

    EXPECT_EQ(points[2](0), 4);
    EXPECT_EQ(points[2](1), 4);

    EXPECT_EQ(points[3](0), 0);
    EXPECT_EQ(points[3](1), 3);
}
