
#include "packages/filter_graph/include/aggregate_sample.h"
#include "packages/filter_graph/include/transform_filter.h"
#include "packages/hal/proto/camera_sample.pb.h"

#include <queue>

namespace stereo {

class SyncMux : public filter_graph::TransformFilter {
public:
    SyncMux(const std::string& leftStreamId, const std::string& rightStreamId);
    ~SyncMux();
    SyncMux(const SyncMux&) = delete;
    SyncMux(const SyncMux&&) = delete;
    SyncMux& operator=(const SyncMux&) = delete;
    SyncMux& operator=(const SyncMux&&) = delete;

    void receive(std::shared_ptr<filter_graph::Container> container) override;

private:
    const std::string m_leftStreamId;
    const std::string m_rightStreamId;

    std::queue<std::shared_ptr<filter_graph::AggregateSample<hal::CameraSample> > > m_leftQueue;
    std::queue<std::shared_ptr<filter_graph::AggregateSample<hal::CameraSample> > > m_rightQueue;
};
}