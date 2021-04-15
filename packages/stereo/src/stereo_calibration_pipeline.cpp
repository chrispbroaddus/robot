#include "packages/stereo/include/stereo_calibration_pipeline.h"

using namespace stereo;

StereoCalibrationPipeline::StereoCalibrationPipeline(const std::string& leftServerAddr, const std::string& leftTopic,
    const std::string& rightServerAddr, const std::string& rightTopic, const std::string& outputFolder)
    : m_leftStreamId(leftTopic)
    , m_rightStreamId(rightTopic)
    , m_sourceFilterThreadPool(1)
    , m_filterThreadPool(2) {

    m_stereoVideoSource = std::make_shared<stereo::ZmqCameraSourceFilter>(
        leftServerAddr, leftTopic, m_leftStreamId, rightServerAddr, rightTopic, m_rightStreamId);
    m_syncMux = std::make_shared<stereo::SyncMux>(m_leftStreamId, m_rightStreamId);
    m_calibrationFileSinkFilter = std::make_shared<stereo::CalibrationFileSinkFilter>(m_leftStreamId, m_rightStreamId, outputFolder);
    wireGraph();
}

StereoCalibrationPipeline::~StereoCalibrationPipeline() {}

void StereoCalibrationPipeline::start() {
    m_sourceFilterThreadPool.start();
    m_filterThreadPool.start();
}

void StereoCalibrationPipeline::stop() {
    m_sourceFilterThreadPool.stop();
    m_filterThreadPool.stop();
}

void StereoCalibrationPipeline::wireGraph() {
    m_stereoVideoSource->attachDownstreamFilter(m_syncMux);
    m_syncMux->attachDownstreamFilter(m_calibrationFileSinkFilter);

    m_sourceFilterThreadPool.attachedFilter(m_stereoVideoSource);

    m_filterThreadPool.attachedFilter(m_syncMux);
    m_filterThreadPool.attachedFilter(m_calibrationFileSinkFilter);

    m_sourceFilterThreadPool.initialize();
    m_filterThreadPool.initialize();
}
