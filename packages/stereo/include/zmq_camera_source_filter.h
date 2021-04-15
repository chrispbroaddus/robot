
#pragma once

#include "packages/filter_graph/include/source_filter.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_topic_sub.h"

namespace stereo {

class ZmqCameraSourceFilter : public filter_graph::SourceFilter {
public:
    ZmqCameraSourceFilter(const std::string& leftServerAddr, const std::string& leftTopic, const std::string& leftStreamId,
        const std::string& rightServerAddr, const std::string& rightTopic, const std::string& rightStreamId);
    ~ZmqCameraSourceFilter();
    ZmqCameraSourceFilter(const ZmqCameraSourceFilter&) = delete;
    ZmqCameraSourceFilter(const ZmqCameraSourceFilter&&) = delete;
    ZmqCameraSourceFilter& operator=(const ZmqCameraSourceFilter&) = delete;
    ZmqCameraSourceFilter& operator=(const ZmqCameraSourceFilter&&) = delete;

    void create() override;

private:
    const std::string m_leftStreamId;
    const std::string m_rightStreamId;

    zmq::context_t m_zmqContext;
    net::ZMQProtobufSubscriber<hal::CameraSample> m_leftCameraSubscriber;
    net::ZMQProtobufSubscriber<hal::CameraSample> m_rightCameraSubscriber;
};
}