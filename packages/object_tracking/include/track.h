#pragma once

#include "packages/core/include/aligned_memory_allocator.h"
#include "packages/core/include/image_view.h"
#include "packages/core/proto/timestamp.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/perception/proto/detection.pb.h"
#include "state.h"

#include <mutex>
#include <queue>

namespace object_tracking {

static uint64_t kObjectTrackingCounter = 1;
static std::mutex kMutexObjectTrackingCounter;

typedef core::ImageView<core::ImageType::uint8> uint8_image_t;

///
/// \brief Base class for a feature used for object tracking
///
class Feature {
public:
    Feature()
        : m_matchFound(false) {}

    virtual ~Feature() = default;

    void setMatchFound(const bool matched) { m_matchFound = matched; }

    bool getMatchFound() const { return m_matchFound; }

private:
    bool m_matchFound;
};

///
/// \brief Class holding a track's feature and metadata for a single frame
///
class TrackMetadataFrame {
public:
    TrackMetadataFrame(const std::shared_ptr<Feature>& feature, const core::HardwareTimestamp& hardwareTimestamp)
        : m_feature(feature)
        , m_hardwareTimestamp(hardwareTimestamp) {}

    std::shared_ptr<Feature> getFeature() { return m_feature; }

private:
    std::shared_ptr<Feature> m_feature;
    core::HardwareTimestamp m_hardwareTimestamp;
};

///
/// \brief Base class for a track describing the state of it and history of features and metadata
///
class Track {

public:
    Track(const int maxNumFrames)
        : m_maxNumFrames(maxNumFrames) {}

    ///
    /// \brief Add new frame's feature and metadata into m_frames
    ///
    void addFrame(const perception::ObjectBoundingBox& box, const uint8_image_t& image, const core::HardwareTimestamp& timestamp) {
        auto feature = extractFeature(box, image);
        std::shared_ptr<TrackMetadataFrame> frame(new TrackMetadataFrame(feature, timestamp));
        frame->getFeature()->setMatchFound(true);
        m_frames.push(frame);

        if ((int)m_frames.size() > m_maxNumFrames) {
            m_frames.pop();
        }
    }

    ///
    /// \brief Return the track ID
    ///
    uint64_t id() { return m_id; }

    ///
    /// \brief True if the state in in object_tracking::ActiveDetectedState
    ///
    bool isActiveDetectedState() {
        if (dynamic_cast<ActiveDetectedState*>(m_state.get())) {
            return true;
        } else {
            return false;
        }
    }

    ///
    /// \brief True if the state in in object_tracking::InactiveState
    ///
    bool isInactiveState() {
        if (dynamic_cast<InactiveState*>(m_state.get())) {
            return true;
        } else {
            return false;
        }
    }

    ///
    /// \brief Update the state of the current frame
    ///
    virtual void update() = 0;

    enum class MatchStatus { NO_MATCH, MATCHED_SELECTED, MATCHED_NOT_SELECTED };

    struct MatchParams {
        virtual ~MatchParams() = default;
    };

    virtual void initializeMatches() {
        auto refFeature = m_frames.back()->getFeature().get();
        refFeature->setMatchFound(false);
    }

    ///
    /// \brief Match a bounding box vs the existing track to decide its association
    ///
    virtual MatchStatus match(const perception::ObjectBoundingBox& box, const uint8_image_t& image, const MatchParams& params) = 0;

protected:
    virtual std::shared_ptr<Feature> extractFeature(const perception::ObjectBoundingBox& box, const uint8_image_t& image) = 0;

    int m_maxNumFrames;
    std::shared_ptr<StateInterface> m_state;
    std::queue<std::shared_ptr<TrackMetadataFrame> > m_frames;
    uint64_t m_id;
};
}
