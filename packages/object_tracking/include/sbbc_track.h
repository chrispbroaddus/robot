#pragma once

#include "sbbc_algorithm.h"
#include "track.h"

namespace object_tracking {

class SimpleBoundingBoxContextTrack : public Track {

public:
    SimpleBoundingBoxContextTrack(const int maxNumFrames)
        : Track(maxNumFrames) {
        m_state.reset(new InitialCandidateState());
        std::lock_guard<std::mutex> lock(kMutexObjectTrackingCounter);
        m_id = kObjectTrackingCounter++;
    }

    void update() {
        auto newState = m_state->transit(std::bind(&SimpleBoundingBoxContextTrackingAlgorithm::track, m_state, m_frames));
        m_state.reset();
        m_state = std::move(newState);
    }

    MatchStatus match(const perception::ObjectBoundingBox& box, const uint8_image_t& image, const MatchParams& params) {
        return SimpleBoundingBoxContextTrackingAlgorithm::match(box, image, m_frames, params);
    }

private:
    std::shared_ptr<Feature> extractFeature(const perception::ObjectBoundingBox& box, const uint8_image_t& image) {
        return SimpleBoundingBoxContextTrackingAlgorithm::extractFeature(box, image);
    }
};
}
