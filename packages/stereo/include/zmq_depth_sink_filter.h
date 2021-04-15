
#pragma once

#include "packages/filter_graph/include/sink_filter.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_topic_pub.h"

namespace stereo {

class ZmqDepthSinkFilter : public filter_graph::SinkFilter {
public:
    ZmqDepthSinkFilter(const std::string& depthStreamId, const std::string& serverAddress, const std::string& depthTopic);
    ~ZmqDepthSinkFilter();
    ZmqDepthSinkFilter(const ZmqDepthSinkFilter&) = delete;
    ZmqDepthSinkFilter(const ZmqDepthSinkFilter&&) = delete;
    ZmqDepthSinkFilter& operator=(const ZmqDepthSinkFilter&) = delete;
    ZmqDepthSinkFilter& operator=(const ZmqDepthSinkFilter&&) = delete;

    void receive(std::shared_ptr<filter_graph::Container> container) override;

private:
    const std::string m_depthStreamId;
    const std::string m_depthTopic;
    zmq::context_t m_context;
    net::ZMQProtobufPublisher<hal::CameraSample> m_depthPublisher;
};
}