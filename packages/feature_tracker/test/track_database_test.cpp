
#include "packages/feature_tracker/include/track_database.h"
#include "packages/feature_tracker/include/track_database_details.h"
#include "gtest/gtest.h"

using namespace feature_tracker;

using frame_id = int64_t;
using track_id = int64_t;

using point_type = feature_tracker::FeaturePoint<track_id>;
using frame_type = feature_tracker::Frame<frame_id, track_id, point_type>;
using track_type = feature_tracker::FeatureTrack<frame_id, track_id, double>;

using track_database_type = TrackDatabase<frame_id, track_id, frame_type, track_type, point_type>;

TEST(TrackDatabase, insertFrame) {
    track_database_type db;

    frame_type frame(10);
    db.insertFrame(frame);

    // If we try to insert the frame again it should throw an exception
    ASSERT_THROW(db.insertFrame(frame), std::runtime_error);
}

TEST(TrackDatabase, insertPoint) {
    track_database_type db;

    constexpr track_id trackId = 100;
    point_type featurePoint(trackId, Eigen::Vector2f(43, 87));

    db.insertFeatureTrack(track_type(trackId));

    // Exception should be thrown if we try to add a point to an invalid frame
    constexpr frame_id frameId = 10;
    ASSERT_THROW(db.insertFeaturePoint(frameId, featurePoint), std::runtime_error);

    db.insertFrame(frame_type(frameId));

    // Should throw on insertion of same point
    ASSERT_NO_THROW(db.insertFeaturePoint(frameId, featurePoint));

    const point_type& point = db.getFeaturePoint(frameId, trackId);
    EXPECT_EQ(trackId, point.m_trackId);
}

TEST(TrackDatabase, insertFeatureTrack) {
    track_database_type db;

    constexpr track_id trackId = 100;
    track_type track(trackId);
    db.insertFeatureTrack(track);

    // If we try to insert the track again it should throw an exception
    ASSERT_THROW(db.insertFeatureTrack(track), std::runtime_error);
}

TEST(TrackDatabase, eraseFrame) {
    track_database_type db;

    constexpr frame_id frameId1 = 10;
    constexpr frame_id frameId2 = 11;

    constexpr track_id trackId1 = 100;
    constexpr track_id trackId2 = 101;

    db.insertFeatureTrack(track_type(trackId1));
    db.insertFeatureTrack(track_type(trackId2));

    db.insertFrame(frame_type(frameId1));
    db.insertFrame(frame_type(frameId2));

    db.insertFeaturePoint(frameId1, point_type(trackId1, Eigen::Vector2f(320, 240)));
    db.insertFeaturePoint(frameId2, point_type(trackId2, Eigen::Vector2f(320, 240)));

    EXPECT_TRUE(db.frameExists(frameId1));

    auto& track1 = db.getTrack(trackId1);
    EXPECT_TRUE(track1.m_frames.find(frameId1) != track1.m_frames.end());

    db.eraseFrame(frameId1);
    EXPECT_FALSE(db.frameExists(frameId1));
    EXPECT_TRUE(db.getFrames().find(frameId1) == db.getFrames().end());
}

TEST(TrackDatabase, eraseFeatureTrack) {
    track_database_type db;

    constexpr frame_id frameId1 = 10;
    constexpr frame_id frameId2 = 11;

    constexpr track_id trackId1 = 100;
    constexpr track_id trackId2 = 101;

    db.insertFeatureTrack(track_type(trackId1));
    db.insertFeatureTrack(track_type(trackId2));

    db.insertFrame(frame_type(frameId1));
    db.insertFrame(frame_type(frameId2));

    db.insertFeaturePoint(frameId1, point_type(trackId1, Eigen::Vector2f(320, 240)));
    db.insertFeaturePoint(frameId2, point_type(trackId2, Eigen::Vector2f(320, 240)));

    EXPECT_TRUE(db.featureTrackExists(trackId1));

    db.eraseFeatureTrack(trackId1);

    EXPECT_FALSE(db.featureTrackExists(trackId1));

    auto& frame1 = db.getFrame(frameId1);
    EXPECT_TRUE(frame1.m_points.find(trackId1) == frame1.m_points.end());
}
