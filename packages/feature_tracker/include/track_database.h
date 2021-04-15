#pragma once

#include "packages/feature_tracker/include/track_database_details.h"

#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace feature_tracker {

/// This is a data structure to store the bipartite graph of frames and tracks. The requirements for the interface
/// are the following:
///
/// 1) Given a track ID, get all the frames that can observe it
/// 2) Given a frame and track ID, get the 2D observation in the image
/// 3) Erase a track from the database
/// 4) Erase a frame from the database
template <typename FRAME_ID_T, typename TRACK_ID_T, typename FRAME_T, typename TRACK_T, typename POINT_T> class TrackDatabase {
public:
    using frame_id = FRAME_ID_T;
    using track_id = TRACK_ID_T;
    using frame_type = FRAME_T;
    using track_type = TRACK_T;
    using point_type = POINT_T;

    TrackDatabase() = default;
    ~TrackDatabase() = default;

    /// Insert a frame in the database.
    /// \param frame The frame to insert
    void insertFrame(frame_type frame) {
        if (m_frames.find(frame.m_frameId) != m_frames.end()) {
            throw std::runtime_error("frame id already exists");
        }

        m_frames[frame.m_frameId] = frame;
    }

    /// Insert a point into the database.
    /// \param frameId The frame ID associated with the point
    /// \param point The point which is being inserted
    void insertFeaturePoint(const frame_id frameId, const point_type& point) {
        auto frameIter = m_frames.find(frameId);
        if (frameIter == m_frames.end()) {
            throw std::runtime_error("frame id doesn't exist");
        }

        // Store the point in the frame
        auto pointIter = frameIter->second.m_points.find(point.m_trackId);
        if (pointIter != frameIter->second.m_points.end()) {
            throw std::runtime_error("point already exists");
        }
        frameIter->second.m_points[point.m_trackId] = point;

        // Link the track to the frame
        auto trackIter = m_tracks.find(point.m_trackId);
        if (trackIter == m_tracks.end()) {
            throw std::runtime_error("track id doens't exist");
        }
        trackIter->second.m_frames.insert(frameId);
    }

    /// Insert a feature track into the database.
    /// \param track The track to insert
    void insertFeatureTrack(const track_type& track) {
        if (m_tracks.find(track.m_trackId) != m_tracks.end()) {
            throw std::runtime_error("track id already exists");
        }

        m_tracks[track.m_trackId] = track;
    }

    /// Get a set of frame ID's that observe the track ID
    /// \param trackId Query track ID that frames are associated with
    /// \return Set of frame ID's that can observe the track ID
    const std::unordered_set<frame_id>& getObservingFrames(const track_id trackId) const {
        auto iter = m_tracks.find(trackId);
        if (iter == m_tracks.end()) {
            throw std::runtime_error("unable to find track id");
        }
        return iter->second.m_frames;
    }

    /// Get the feature point associated with trackId at frameId
    /// \param frameId Query frame ID
    /// \param trackId Query track ID
    /// \return
    point_type getFeaturePoint(const int frameId, const int trackId) const {
        auto frameIter = m_frames.find(frameId);
        if (frameIter == m_frames.end()) {
            throw std::runtime_error("unable to find frame id");
        }

        auto pointIter = frameIter->second.m_points.find(trackId);
        if (pointIter == frameIter->second.m_points.end()) {
            throw std::runtime_error("unable to find track id");
        }
        return pointIter->second;
    }

    /// Erase a frame from the database
    /// \param frameId Frame ID to erase
    void eraseFrame(const frame_id frameId) {
        auto frameIt = m_frames.find(frameId);
        if (frameIt == m_frames.end()) {
            throw std::runtime_error("frame id doesn't exist");
        }

        // Erase the frame ID from all the tracks
        for (auto trackIter = m_tracks.begin(); trackIter != m_tracks.end();) {
            trackIter->second.m_frames.erase(frameId);
            // If the track isn't observed by any frames then erase the track
            if (trackIter->second.m_frames.size() == 0) {
                trackIter = m_tracks.erase(trackIter);
            } else {
                trackIter++;
            }
        }

        m_frames.erase(frameIt);
    }

    /// Erase a track from the database
    /// \param trackId Track ID to erase
    void eraseFeatureTrack(const track_id trackId) {
        auto trackIter = m_tracks.find(trackId);
        if (trackIter == m_tracks.end()) {
            throw std::runtime_error("track id doesn't exist");
        }

        // Erase all the points associated with the track ID in the frame
        for (auto frameId : trackIter->second.m_frames) {
            auto& frame = m_frames[frameId];
            if (frame.m_points.erase(trackId) == 0) {
                throw std::runtime_error("track id isn't in the points set");
            }
        }

        m_tracks.erase(trackIter);
    }

    /// Get a frame from an ID
    /// \param frameId Frame ID to get
    /// \return Frame ID structure
    frame_type& getFrame(const frame_id frameId) {
        auto frameIter = m_frames.find(frameId);
        if (frameIter == m_frames.end()) {
            throw std::runtime_error("frame is doesn't exist");
        }
        return frameIter->second;
    }
    const frame_type& getFrame(const frame_id frameId) const {
        auto frameIter = m_frames.find(frameId);
        if (frameIter == m_frames.end()) {
            throw std::runtime_error("frame is doesn't exist");
        }
        return frameIter->second;
    }

    /// Get a track from an ID
    /// \param trackId Track ID to get
    /// \return Frack ID structure
    track_type& getTrack(const track_id trackId) {
        auto trackIter = m_tracks.find(trackId);
        if (trackIter == m_tracks.end()) {
            throw std::runtime_error("track is doesn't exist");
        }
        return trackIter->second;
    }
    const track_type& getTrack(const track_id trackId) const {
        auto trackIter = m_tracks.find(trackId);
        if (trackIter == m_tracks.end()) {
            throw std::runtime_error("track is doesn't exist");
        }
        return trackIter->second;
    }

    /// Check if a frame ID exists in the database
    /// \param frameId Frame ID to check
    /// \return True if the frame exists, false otherwise
    bool frameExists(const frame_id frameId) const { return m_frames.find(frameId) != m_frames.end(); }

    /// Check if a track ID exist in the database
    /// \param trackId Track ID to check
    /// \return True if the track ID exists, false otherwise
    bool featureTrackExists(const track_id trackId) const { return m_tracks.find(trackId) != m_tracks.end(); }

    /// Get all the tracks
    const std::unordered_map<track_id, track_type>& getTracks() const { return m_tracks; }

    /// Get all the tracks
    std::unordered_map<track_id, track_type>& getTracks() { return m_tracks; }

    /// Get all the frames
    const std::unordered_map<frame_id, frame_type>& getFrames() const { return m_frames; };

private:
    /// Map of tracks indexed by track ID
    std::unordered_map<track_id, track_type> m_tracks;

    /// Map of frames indexed by frame ID
    std::unordered_map<frame_id, frame_type> m_frames;
};
}
