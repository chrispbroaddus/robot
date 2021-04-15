
#include "packages/feature_tracker/include/stereo_feature_track_index.h"
#include "gtest/gtest.h"

using namespace feature_tracker;

/// Add a frame with no matches, and check that there are no reliable tracks
TEST(FeatureTrackIndex, firstFrame) {
    details::stereo_feature_track_index_type index(5);

    std::vector<feature_detectors::FeaturePoint> points;
    std::vector<std::pair<int, int> > matches;

    feature_detectors::FeaturePoint point1;
    point1.set_x(10);
    point1.set_y(20);

    feature_detectors::FeaturePoint point2;
    point2.set_x(20);
    point2.set_y(30);

    points.push_back(point1);
    points.push_back(point2);

    index.addMatches(points, matches);

    EXPECT_EQ(0, index.getTracksOfMinLength(3).size());
}

TEST(FeatureTrackIndex, oneGoodTrack) {
    details::stereo_feature_track_index_type index(3);

    {
        std::vector<feature_detectors::FeaturePoint> points;
        std::vector<std::pair<int, int> > matches;

        feature_detectors::FeaturePoint point1;
        point1.set_x(10);
        point1.set_y(20);

        feature_detectors::FeaturePoint point2;
        point2.set_x(20);
        point2.set_y(30);

        points.push_back(point1);
        points.push_back(point2);

        index.addMatches(points, matches);
    }

    EXPECT_EQ(0, index.getTracksOfMinLength(3).size());

    {
        std::vector<feature_detectors::FeaturePoint> points;
        std::vector<std::pair<int, int> > matches;

        feature_detectors::FeaturePoint point1;
        point1.set_x(11);
        point1.set_y(21);

        feature_detectors::FeaturePoint point2;
        point2.set_x(30);
        point2.set_y(40);

        points.push_back(point1);
        points.push_back(point2);

        matches.push_back(std::make_pair(0, 0));

        index.addMatches(points, matches);
    }

    EXPECT_EQ(0, index.getTracksOfMinLength(3).size());

    {
        std::vector<feature_detectors::FeaturePoint> points;
        std::vector<std::pair<int, int> > matches;

        feature_detectors::FeaturePoint point1;
        point1.set_x(12);
        point1.set_y(22);

        feature_detectors::FeaturePoint point2;
        point2.set_x(31);
        point2.set_y(41);

        points.push_back(point1);
        points.push_back(point2);

        matches.push_back(std::make_pair(0, 0));
        matches.push_back(std::make_pair(1, 1));

        index.addMatches(points, matches);
    }

    EXPECT_EQ(1, index.getTracksOfMinLength(3).size()); // should have 1 track of length 3
    EXPECT_EQ(0, index.getTracksOfMinLength(3).front()); // track ID should be 0

    EXPECT_EQ(2, index.getTracksOfMinLength(2).size()); // should have 2 tracks of length 2

    {
        std::vector<feature_detectors::FeaturePoint> points;
        std::vector<std::pair<int, int> > matches;

        feature_detectors::FeaturePoint point1;
        point1.set_x(13);
        point1.set_y(23);

        feature_detectors::FeaturePoint point2;
        point2.set_x(32);
        point2.set_y(42);

        points.push_back(point1);
        points.push_back(point2);

        matches.push_back(std::make_pair(0, 0));
        matches.push_back(std::make_pair(1, 1));

        index.addMatches(points, matches);
    }

    EXPECT_EQ(2, index.getTracksOfMinLength(3).size()); // should have 2 tracks of length 3
}
