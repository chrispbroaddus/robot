
#include "packages/stereo/include/stereo_pipeline.h"

using namespace stereo;

StereoPipeline::StereoPipeline(const std::string& leftServerAddr, const std::string& leftTopic, const std::string& rightServerAddr,
    const std::string& rightTopic, const std::string& systemCalibrationFile, const std::string& depthPublisherAddress,
    const std::string& depthTopic, const bool& outputPointCloud)
    : m_leftStreamId(leftTopic)
    , m_rightStreamId(rightTopic)
    , m_depthStreamId(depthTopic)
    , m_sourceFilterThreadPool(1)
    , m_filterThreadPool(2) {

    m_stereoVideoSource = std::make_shared<stereo::ZmqCameraSourceFilter>(
        leftServerAddr, leftTopic, m_leftStreamId, rightServerAddr, rightTopic, m_rightStreamId);
    m_syncMux = std::make_shared<stereo::SyncMux>(m_leftStreamId, m_rightStreamId);
    m_stereoFilter
        = std::make_shared<stereo::StereoFilter>(m_leftStreamId, m_rightStreamId, m_depthStreamId, systemCalibrationFile, outputPointCloud);
    m_depthSinkFilter = std::make_shared<stereo::ZmqDepthSinkFilter>(m_depthStreamId, depthPublisherAddress, depthTopic);
    wireGraph();
}

StereoPipeline::~StereoPipeline() {}

void StereoPipeline::start() {
    m_sourceFilterThreadPool.start();
    m_filterThreadPool.start();
}

void StereoPipeline::stop() {
    m_sourceFilterThreadPool.stop();
    m_filterThreadPool.stop();
}

void StereoPipeline::wireGraph() {
    m_stereoVideoSource->attachDownstreamFilter(m_syncMux);
    m_syncMux->attachDownstreamFilter(m_stereoFilter);
    m_stereoFilter->attachDownstreamFilter(m_depthSinkFilter);

    m_sourceFilterThreadPool.attachedFilter(m_stereoVideoSource);

    m_filterThreadPool.attachedFilter(m_syncMux);
    m_filterThreadPool.attachedFilter(m_stereoFilter);
    m_filterThreadPool.attachedFilter(m_depthSinkFilter);

    m_sourceFilterThreadPool.initialize();
    m_filterThreadPool.initialize();
}
