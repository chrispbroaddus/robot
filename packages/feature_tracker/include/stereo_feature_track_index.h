
#pragma once

#include "packages/feature_tracker/include/feature_track_index.h"

namespace feature_tracker {
namespace details {
    /// Represents the track of a feature in the world and the ID assigned by the stereo tracker.
    template <typename FRAME_ID_T, typename TRACK_ID_T, typename POINT_T>
    struct StereoFeatureTrack : public FeatureTrack<FRAME_ID_T, TRACK_ID_T, POINT_T> {
        using frame_id = FRAME_ID_T;
        using track_id = TRACK_ID_T;
        using point_type = POINT_T;
        using Super = FeatureTrack<FRAME_ID_T, TRACK_ID_T, POINT_T>;

        StereoFeatureTrack()
            : Super()
            , m_stereoTrackId(-1) {}
        StereoFeatureTrack(const track_id trackId)
            : Super(trackId)
            , m_stereoTrackId(-1) {}

        /// The ID assigned by the stereo matcher
        track_id m_stereoTrackId;
    };

    using frame_id = int32_t;
    using track_id = int32_t;

    using point_type = feature_tracker::FeaturePoint<track_id>;
    using frame_type = feature_tracker::Frame<frame_id, track_id, point_type>;
    using track_type = StereoFeatureTrack<frame_id, track_id, double>;

    using stereo_feature_track_index_type = FeatureTrackIndex<frame_id, track_id, frame_type, track_type, point_type>;
}
}
