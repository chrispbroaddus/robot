
#pragma once

#include "packages/feature_tracker/include/stereo_feature_track_index.h"

namespace feature_tracker {
/// The stereo tracker assigns track ID's to the left and right camera tracks
class StereoFeatureTracker {
public:
    using track_id = details::stereo_feature_track_index_type::track_id;

    StereoFeatureTracker(const size_t minTrackLength)
        : m_minTrackLength(minTrackLength)
        , m_nextStereoTrackId(0) {}
    ~StereoFeatureTracker() = default;

    /// Assign a stereo track ID to the left and right tracks.
    /// \param leftFeatureTrackIndex Left camera track index
    /// \param rightFeatureTrackIndex Right camera track index
    /// \param matches The matches from left/right cameras
    /// \param leftFeatureStoreTrackIds The track ID's of left camera
    /// \param rightFeatureStoreTrackIds The track ID's of the right camera
    /// \return The number of tracks assigned
    size_t assignStereoTrackIds(details::stereo_feature_track_index_type& leftFeatureTrackIndex,
        details::stereo_feature_track_index_type& rightFeatureTrackIndex, const std::vector<std::pair<int, int> >& matches,
        const std::vector<track_id>& leftFeatureStoreTrackIds, const std::vector<track_id>& rightFeatureStoreTrackIds) {
        size_t numAssignedTracks = 0;
        for (const auto& match : matches) {
            auto& leftTrack = leftFeatureTrackIndex.getTrack(leftFeatureStoreTrackIds[match.first]);
            auto& rightTrack = rightFeatureTrackIndex.getTrack(rightFeatureStoreTrackIds[match.second]);
            // If the tracks have not been assigned a stereo ID and are >= minTrackLength, then assign a stereo track ID
            if (leftTrack.m_stereoTrackId < 0 && rightTrack.m_stereoTrackId < 0 && leftTrack.m_frames.size() >= m_minTrackLength
                && rightTrack.m_frames.size() >= m_minTrackLength) {
                leftTrack.m_stereoTrackId = m_nextStereoTrackId;
                rightTrack.m_stereoTrackId = m_nextStereoTrackId;
                m_nextStereoTrackId++;
                numAssignedTracks++;
            }
        }
        return numAssignedTracks;
    }

private:
    /// Minimum length of a track before being assigned a stereo track ID
    const size_t m_minTrackLength;
    /// Store the ID of the next stereo track ID
    track_id m_nextStereoTrackId;
};
}
