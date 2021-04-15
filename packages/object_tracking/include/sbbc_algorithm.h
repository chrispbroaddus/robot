#pragma once

#include "algorithm.h"
#include "packages/machine_learning/include/detection_utils.h"

namespace object_tracking {

///
/// \details Used CRTP trick, https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
///
class SimpleBoundingBoxContextTrackingAlgorithm : public TrackingAlgorithm<SimpleBoundingBoxContextTrackingAlgorithm> {

public:
    class SBBCFeature : public object_tracking::Feature {
    public:
        SBBCFeature(const perception::ObjectBoundingBox& box)
            : Feature()
            , m_box(box) {}

        ~SBBCFeature() {}

        perception::ObjectBoundingBox getBox() const { return m_box; }

    private:
        perception::ObjectBoundingBox m_box;
    };

    using frame_t = TrackMetadataFrame;

    static std::shared_ptr<SBBCFeature> extractFeature(const perception::ObjectBoundingBox& box, const uint8_image_t& /*image*/) {
        return std::make_shared<SBBCFeature>(box);
    }

    struct MatchParams : public Track::MatchParams {
        float iouThreshold;
    };

    ///
    /// \brief Compare the intersection-over-ratio between the inspection box vs
    ///        the reference box.
    ///
    static Track::MatchStatus match(const perception::ObjectBoundingBox& box, const uint8_image_t& /*image*/,
        std::queue<std::shared_ptr<TrackMetadataFrame> >& frames, const Track::MatchParams& params) {
        auto refFeature = dynamic_cast<SBBCFeature*>(frames.back()->getFeature().get());
        auto refBox = refFeature->getBox();
        if (!object_detection::compareBoundingBoxPairClass(refBox, box)) {
            return Track::MatchStatus::NO_MATCH;
        }
        auto sbbcCompareParams = static_cast<const MatchParams*>(&params);
        if (object_detection::computeBoundingBoxPairIouRatio(refBox, box) < sbbcCompareParams->iouThreshold) {
            return Track::MatchStatus::NO_MATCH;
        }
        if (refFeature->getMatchFound()) {
            return Track::MatchStatus::MATCHED_NOT_SELECTED;
        }
        refFeature->setMatchFound(true);
        return Track::MatchStatus::MATCHED_SELECTED;
    }

    ///
    /// \brief In SBBC algorithm, by definition, we accept all the detection candidates.
    ///        For more complicated algorith, we can apply Non-Maximum-Suppression, for example.
    ///
    static std::shared_ptr<StateInterface> trackFromInitialCandidateState(
        const std::shared_ptr<StateInterface> /*state*/, const std::queue<std::shared_ptr<frame_t> >& /*frames*/) {
        return std::make_shared<ActiveDetectedState>();
    }

    ///
    /// \brief If there is a bounding box found on the current frame, transit to ActiveDetectedState again,
    ///        If not, transit to ActiveLostState.
    ///
    static std::shared_ptr<StateInterface> trackFromActiveDetectedState(
        const std::shared_ptr<StateInterface> /*state*/, const std::queue<std::shared_ptr<frame_t> >& frames) {
        auto refFeature = dynamic_cast<SBBCFeature*>(frames.back()->getFeature().get());
        if (refFeature->getMatchFound()) {
            return std::make_shared<ActiveDetectedState>();
        } else {
            return std::make_shared<ActiveLostState>();
        }
    }

    ///
    /// \brief In SBBC algorithm, any ActiveLostState track trasits to InactiveState..
    ///
    static std::shared_ptr<StateInterface> trackFromActiveLostState(
        const std::shared_ptr<StateInterface> /*state*/, const std::queue<std::shared_ptr<frame_t> >& /*frames*/) {
        return std::make_shared<InactiveState>();
    }

    ///
    /// \brief The track came in to the InactiveState should be deleted immediately,
    ///        so there shouldn't exist any track trying to transit *from* InactiveState.
    ///
    static std::shared_ptr<StateInterface> trackFromInactiveState(
        const std::shared_ptr<StateInterface> /*state*/, const std::queue<std::shared_ptr<frame_t> >& /*frames*/) {
        throw std::runtime_error("Error in deleting InactiveState.");
    }
};
}