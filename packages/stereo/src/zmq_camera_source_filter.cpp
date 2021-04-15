
#include "packages/stereo/include/zmq_camera_source_filter.h"
#include "packages/filter_graph/include/aggregate_sample.h"

using namespace stereo;

ZmqCameraSourceFilter::ZmqCameraSourceFilter(const std::string& leftServerAddr, const std::string& leftTopic,
    const std::string& leftStreamId, const std::string& rightServerAddr, const std::string& rightTopic, const std::string& rightStreamId)
    : filter_graph::SourceFilter("ZmqCameraSourceFilter", 10)
    , m_leftStreamId(leftStreamId)
    , m_rightStreamId(rightStreamId)
    , m_zmqContext(1)
    , m_leftCameraSubscriber(m_zmqContext, leftServerAddr, leftTopic, 1)
    , m_rightCameraSubscriber(m_zmqContext, rightServerAddr, rightTopic, 1) {}

ZmqCameraSourceFilter::~ZmqCameraSourceFilter() {}

void ZmqCameraSourceFilter::create() {
    constexpr std::chrono::milliseconds timeout(1);

    auto container = std::make_shared<filter_graph::Container>();

    if (m_leftCameraSubscriber.poll(timeout)) {
        auto sample = std::make_shared<filter_graph::AggregateSample<hal::CameraSample> >(m_leftStreamId);
        if (m_leftCameraSubscriber.recv(sample->data())) {
            container->add(sample->streamId(), sample);
        } else {
            LOG(ERROR) << "Left image recv failed";
        }
    }
    if (m_rightCameraSubscriber.poll(timeout)) {
        auto sample = std::make_shared<filter_graph::AggregateSample<hal::CameraSample> >(m_rightStreamId);
        if (m_rightCameraSubscriber.recv(sample->data())) {
            container->add(sample->streamId(), sample);
        } else {
            LOG(ERROR) << "Right image recv failed";
        }
    }

    if (container->size()) {
        getOutputQueue()->enqueue(container);
        send();
    }
}