
#include "packages/feature_tracker/include/stereo_feature_tracker.h"
#include "packages/feature_tracker/include/ncc_feature_store.h"

#include "gtest/gtest.h"

using namespace core;

TEST(StereoFeatureTracker, assignStereoTrackIds) {
    using track_id = feature_tracker::details::stereo_feature_track_index_type::track_id;

    feature_tracker::details::stereo_feature_track_index_type leftFeatureTrackIndex(10);
    feature_tracker::details::stereo_feature_track_index_type rightFeatureTrackIndex(10);

    std::vector<feature_detectors::FeaturePoint> leftPoints;
    feature_detectors::FeaturePoint leftPoint1;
    leftPoint1.set_x(20);
    leftPoint1.set_y(20);
    feature_detectors::FeaturePoint leftPoint2;
    leftPoint2.set_x(100);
    leftPoint2.set_y(100);
    leftPoints.push_back(leftPoint1);
    leftPoints.push_back(leftPoint2);
    std::vector<std::pair<int, int> > leftMatches;
    const std::vector<track_id> leftTracksIds = leftFeatureTrackIndex.addMatches(leftPoints, leftMatches);

    EXPECT_EQ(2, leftTracksIds.size());

    std::vector<feature_detectors::FeaturePoint> rightPoints;
    feature_detectors::FeaturePoint rightPoint1;
    rightPoint1.set_x(20);
    rightPoint1.set_y(20);
    feature_detectors::FeaturePoint rightPoint2;
    rightPoint2.set_x(100);
    rightPoint2.set_y(100);
    rightPoints.push_back(rightPoint1);
    rightPoints.push_back(rightPoint2);
    std::vector<std::pair<int, int> > rightMatches;
    const std::vector<track_id> rightTrackIds = rightFeatureTrackIndex.addMatches(rightPoints, rightMatches);

    EXPECT_EQ(2, rightTrackIds.size());

    std::vector<std::pair<int, int> > stereoMatches;
    stereoMatches.push_back(std::make_pair(0, 0));
    stereoMatches.push_back(std::make_pair(1, 1));

    feature_tracker::StereoFeatureTracker tracker(0);
    const size_t numAssignedTracks
        = tracker.assignStereoTrackIds(leftFeatureTrackIndex, rightFeatureTrackIndex, stereoMatches, leftTracksIds, rightTrackIds);

    EXPECT_EQ(2, numAssignedTracks);

    EXPECT_EQ(0, leftFeatureTrackIndex.getTrack(leftTracksIds[0]).m_stereoTrackId);
    EXPECT_EQ(0, rightFeatureTrackIndex.getTrack(rightTrackIds[0]).m_stereoTrackId);

    EXPECT_EQ(1, leftFeatureTrackIndex.getTrack(leftTracksIds[1]).m_stereoTrackId);
    EXPECT_EQ(1, rightFeatureTrackIndex.getTrack(rightTrackIds[1]).m_stereoTrackId);
}
