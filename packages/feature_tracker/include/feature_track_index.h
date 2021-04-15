
#pragma once

#include "packages/feature_detectors/proto/feature_point.pb.h"
#include "packages/feature_tracker/include/track_database.h"
#include "packages/feature_tracker/include/track_database_details.h"
#include <cstdint>
#include <list>
#include <stdexcept>

namespace feature_tracker {

/// The frame-to-frame feature tracker keeps track of the matches from the previous and current frame. We need to maintain
/// these links over time in a sliding window. The track index keeps the frame and track information over a
/// "maxFrameSize" sliding window to discover the reliable tracks, i.e. those points that are consistently matched
/// over a minimum number of frames.
template <typename FRAME_ID_T, typename TRACK_ID_T, typename FRAME_T, typename TRACK_T, typename POINT_T>
class FeatureTrackIndex : public feature_tracker::TrackDatabase<FRAME_ID_T, TRACK_ID_T, FRAME_T, TRACK_T, POINT_T> {
public:
    using Super = feature_tracker::TrackDatabase<FRAME_ID_T, TRACK_ID_T, FRAME_T, TRACK_T, POINT_T>;
    using frame_id = FRAME_ID_T;
    using track_id = TRACK_ID_T;
    using frame_type = FRAME_T;
    using track_type = TRACK_T;
    using point_type = POINT_T;

    FeatureTrackIndex(const size_t maxFrameSize)
        : Super()
        , m_maxFrameSize(maxFrameSize)
        , m_nextFrameId(0)
        , m_nextTrackId(0) {
        if (m_maxFrameSize == 0) {
            throw std::runtime_error("max frames must be > 0");
        }
    }
    ~FeatureTrackIndex() = default;

    /// Add points and matches from the feature tracker to the track index.
    /// \param points The point from the tracker
    /// \param matches The matches from previous (first) to current (second) frame
    /// \return Vector of track ID's for each point
    std::vector<track_id> addMatches(
        const std::vector<feature_detectors::FeaturePoint>& points, const std::vector<std::pair<int, int> >& matches) {

        if (m_frameIds.size() == m_maxFrameSize) {
            Super::eraseFrame(m_frameIds.front());
            m_frameIds.pop_front();
        }

        // Add the frame to the database
        Super::insertFrame(frame_type(m_nextFrameId));

        // We need to keep track of point assignment to track ID's. Here we initialize a vector to -1 and then in the
        // loop below we set the track ID for each point.
        std::vector<track_id> trackIds(points.size(), -1);

        // Loop over all the matches and add the observation to the track database.
        for (size_t i = 0; i < matches.size(); i++) {
            const track_id& trackId = m_prevTrackIds[matches[i].first];
            if (trackId < 0) {
                throw std::runtime_error("track id isn't valid");
            }

            // Get the track from the database and add observation
            const feature_detectors::FeaturePoint& point = points[matches[i].second];
            Super::insertFeaturePoint(m_nextFrameId, point_type(trackId, point));

            trackIds[matches[i].second] = trackId;
        }

        // Loop over all the points and create new tracks
        for (size_t i = 0; i < points.size(); i++) {
            if (trackIds[i] < 0) {
                Super::insertFeatureTrack(track_type(m_nextTrackId));
                Super::insertFeaturePoint(m_nextFrameId, point_type(m_nextTrackId, points[i]));
                trackIds[i] = m_nextTrackId;
                m_nextTrackId++;
            }
        }

        m_frameIds.push_back(m_nextFrameId);
        m_prevTrackIds = trackIds;
        m_nextFrameId++;

        return trackIds;
    }

    /// Get tracks of minimum tracks
    /// \param minTrackSize Minimum length
    /// \return The tracks of minimum track length
    std::vector<track_id> getTracksOfMinLength(const size_t minTrackSize) const {
        std::vector<track_id> minLengthTracks;
        const std::unordered_map<track_id, track_type>& tracks = Super::getTracks();
        for (auto& track : tracks) {
            if (track.second.m_frames.size() >= minTrackSize) {
                minLengthTracks.push_back(track.second.m_trackId);
            }
        }
        return minLengthTracks;
    }

    /// \return Get the newest frame ID
    frame_id getNewestFrameId() const {
        if (m_frameIds.empty()) {
            throw std::runtime_error("frame id's list empty");
        }
        return m_frameIds.back();
    }

private:
    /// Sliding window of frame ID's
    std::list<frame_id> m_frameIds;

    /// ID's of the previous points
    std::vector<track_id> m_prevTrackIds;

    /// Maximum number of frames stored in the track index
    size_t m_maxFrameSize;

    /// The ID of the next frame
    frame_id m_nextFrameId;

    /// The ID of the next track
    track_id m_nextTrackId;
};
}
