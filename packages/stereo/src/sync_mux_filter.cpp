
#include "packages/stereo/include/sync_mux_filter.h"
#include "glog/logging.h"

using namespace stereo;

SyncMux::SyncMux(const std::string& leftStreamId, const std::string& rightStreamId)
    : filter_graph::TransformFilter("SyncMux", 10, 10)
    , m_leftStreamId(leftStreamId)
    , m_rightStreamId(rightStreamId) {}

SyncMux::~SyncMux() {}

void SyncMux::receive(std::shared_ptr<filter_graph::Container> container) {
    constexpr size_t kDefaultFrameDelay = 3;
    constexpr int kDefaultSyncDurationInMilliseconds = 70;

    auto leftSample = container->get(m_leftStreamId);
    auto rightSample = container->get(m_rightStreamId);

    if (m_leftQueue.size() == kDefaultFrameDelay) {
        m_leftQueue.pop();
    }
    if (m_rightQueue.size() == kDefaultFrameDelay) {
        m_rightQueue.pop();
    }

    // Push the left/right camera samples into left/right queues
    if (leftSample.get()) {
        auto leftCameraSample = std::static_pointer_cast<filter_graph::AggregateSample<hal::CameraSample> >(leftSample);
        m_leftQueue.push(leftCameraSample);
    }
    if (rightSample.get()) {
        auto rightCameraSample = std::static_pointer_cast<filter_graph::AggregateSample<hal::CameraSample> >(rightSample);
        m_rightQueue.push(rightCameraSample);
    }

    while (m_leftQueue.size() && m_rightQueue.size()) {
        const uint64_t leftTimeInNanoseconds = m_leftQueue.front()->data().systemtimestamp().nanos();
        const uint64_t rightTimeInNanoseconds = m_rightQueue.front()->data().systemtimestamp().nanos();
        const int deltaDurationInMilliseconds = (int)(((double)leftTimeInNanoseconds - (double)rightTimeInNanoseconds) / (double)1e6);
        const int absDeltaDurationInMilliseconds = std::abs(deltaDurationInMilliseconds);
        LOG(INFO) << "Left: " << leftTimeInNanoseconds;
        LOG(INFO) << "Right: " << rightTimeInNanoseconds;
        LOG(INFO) << "Delta: " << deltaDurationInMilliseconds;
        LOG(INFO) << "AbsDelta: " << absDeltaDurationInMilliseconds;
        if (absDeltaDurationInMilliseconds < kDefaultSyncDurationInMilliseconds) {
            auto container = std::make_shared<filter_graph::Container>();
            container->add(m_leftQueue.front()->streamId(), m_leftQueue.front());
            container->add(m_rightQueue.front()->streamId(), m_rightQueue.front());
            m_leftQueue.pop();
            m_rightQueue.pop();
            getOutputQueue()->enqueue(container);
            send();
        } else {
            if (deltaDurationInMilliseconds < 0) {
                //  Right is recent, so pop the left
                m_leftQueue.pop();
                LOG(INFO) << "Out of sync: Pop left";
            } else {
                // Left is recent, so pop the right
                m_rightQueue.pop();
                LOG(INFO) << "Out of sync: Pop right";
            }
        }
    }
}