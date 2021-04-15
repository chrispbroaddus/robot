
#include "packages/feature_detectors/include/feature_point_selector.h"
#include "gtest/gtest.h"

TEST(FeaturePointSelector, regression) {
    feature_detectors::FeaturePointSelector featureSelector(16, 16, 2, 2, 4);

    std::vector<feature_detectors::FeaturePoint> points;

    // Insert 1 point in each bucket
    feature_detectors::FeaturePoint p1;
    p1.set_x(0);
    p1.set_y(0);
    p1.set_score(0);
    feature_detectors::FeaturePoint p2;
    p2.set_x(8);
    p2.set_y(0);
    p2.set_score(0);
    feature_detectors::FeaturePoint p3;
    p3.set_x(0);
    p3.set_y(8);
    p3.set_score(0);
    feature_detectors::FeaturePoint p4;
    p4.set_x(8);
    p4.set_y(8);
    p4.set_score(0);

    // Insert an extra point in first bucket
    feature_detectors::FeaturePoint p5;
    p5.set_x(1);
    p5.set_y(1);
    p5.set_score(1);

    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
    points.push_back(p4);
    points.push_back(p5);

    const std::vector<feature_detectors::FeaturePoint>& selectedPoints = featureSelector.select(points);

    EXPECT_EQ(4, selectedPoints.size());
    EXPECT_EQ(1, selectedPoints[0].x());
    EXPECT_EQ(1, selectedPoints[0].y());
    EXPECT_EQ(0, selectedPoints[1].x());
    EXPECT_EQ(8, selectedPoints[1].y());
    EXPECT_EQ(8, selectedPoints[2].x());
    EXPECT_EQ(0, selectedPoints[2].y());
    EXPECT_EQ(8, selectedPoints[3].x());
    EXPECT_EQ(8, selectedPoints[3].y());
}
