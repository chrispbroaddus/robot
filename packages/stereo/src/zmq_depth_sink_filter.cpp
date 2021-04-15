
#include "packages/stereo/include/zmq_depth_sink_filter.h"
#include "packages/filter_graph/include/aggregate_sample.h"

using namespace stereo;

ZmqDepthSinkFilter::ZmqDepthSinkFilter(const std::string& depthStreamId, const std::string& serverAddress, const std::string& depthTopic)
    : filter_graph::SinkFilter("ZmqDepthSinkFilter", 10)
    , m_depthStreamId(depthStreamId)
    , m_depthTopic(depthTopic)
    , m_context(1)
    , m_depthPublisher(m_context, serverAddress, 1, 100) {}

ZmqDepthSinkFilter::~ZmqDepthSinkFilter() {}

void ZmqDepthSinkFilter::receive(std::shared_ptr<filter_graph::Container> container) {
    std::shared_ptr<filter_graph::Sample> depthSample = container->get(m_depthStreamId);
    if (depthSample) {
        auto depthCameraSample = static_cast<filter_graph::AggregateSample<hal::CameraSample>*>(depthSample.get());
        m_depthPublisher.send(depthCameraSample->data(), m_depthTopic);
    }
}
