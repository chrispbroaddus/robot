#pragma once

#include "track.h"

namespace object_tracking {

///
/// \brief The track manager for object tracking for templatized tracking algorithm
///
template <class TRACK_TYPE> class TrackManager {

public:
    TrackManager(const int numMaxFrames, std::shared_ptr<Track::MatchParams> compareParams)
        : m_numMaxFrames(numMaxFrames)
        , m_compareParams(compareParams) {}

    ///
    /// \brief Assign incoming bounding boxes from a current frame into tracks, or create a new track.
    /// \param det The incoming bounding boxes from a current frame
    /// \param cameraSample The camera data to extract context features from.
    ///
    void addFrame(perception::Detection& det, const uint8_image_t& image, const core::HardwareTimestamp& timestamp) {

        // Set match results as false as an initialization
        for (auto track : m_tracks) {
            track->initializeMatches();
        }

        for (int i = 0; i < det.mutable_box_detection()->bounding_boxes_size(); i++) {

            auto box = det.mutable_box_detection()->mutable_bounding_boxes(i);
            if (box->category().type() == perception::Category_CategoryType::Category_CategoryType_UNKNOWN) {
                continue;
            }

            Track::MatchStatus bestStatus = static_cast<Track::MatchStatus>(Track::MatchStatus::NO_MATCH);

            for (auto track : m_tracks) {

                auto compareResult = track->match(*box, image, *m_compareParams.get());
                if (compareResult == Track::MatchStatus::MATCHED_SELECTED) {
                    track->addFrame(*box, image, timestamp);
                    box->set_instance_id(track->id());
                    LOG(INFO) << __PRETTY_FUNCTION__ << " ... found a match to Track ID : " << track->id();
                    bestStatus = Track::MatchStatus::MATCHED_SELECTED;
                } else if (compareResult == Track::MatchStatus::MATCHED_NOT_SELECTED) {
                    LOG(INFO) << __PRETTY_FUNCTION__ << " ... found a match, but not selected : " << track->id();
                    bestStatus = Track::MatchStatus::MATCHED_NOT_SELECTED;
                }
            }

            if (bestStatus == Track::MatchStatus::NO_MATCH) {
                std::shared_ptr<Track> newTrack(new TRACK_TYPE(m_numMaxFrames));
                m_tracks.push_back(newTrack);
                m_tracks.back()->addFrame(*box, image, timestamp);
                LOG(INFO) << __PRETTY_FUNCTION__ << " ... creating a new Track ID : " << m_tracks.back()->id();
                box->set_instance_id(m_tracks.back()->id());
            }
        }

        updateTracks();
    }

    ///
    /// \brief Return the number of tracks
    ///
    size_t trackSize() { return m_tracks.size(); }

    ///
    /// \brief Accessor to a track
    ///
    const std::shared_ptr<Track> track(size_t index) { return m_tracks[index]; }

private:
    ///
    /// \brief Update tracks' states, and delete if not active anymore.
    ///
    void updateTracks() {
        for (size_t i = 0; i < m_tracks.size(); i++) {
            m_tracks[i]->update();
        }
        auto track = std::begin(m_tracks);
        while (track != std::end(m_tracks)) {
            if (track->get()->isInactiveState()) {
                track = m_tracks.erase(track);
            } else {
                ++track;
            }
        }
    }

    int m_numMaxFrames;
    std::shared_ptr<Track::MatchParams> m_compareParams;
    std::vector<std::shared_ptr<Track> > m_tracks;
};
}
