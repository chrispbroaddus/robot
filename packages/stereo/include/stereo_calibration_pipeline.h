#pragma once

#include "packages/stereo/include/calibration_file_sink_filter.h"
#include "packages/stereo/include/sync_mux_filter.h"
#include "packages/stereo/include/zmq_camera_source_filter.h"

namespace stereo {

/// Pipeline for capturing images for stereo calibration
class StereoCalibrationPipeline {
public:
    StereoCalibrationPipeline(const std::string& leftServerAddr, const std::string& leftTopic, const std::string& rightServerAddr,
        const std::string& rightTopic, const std::string& outputFolder);
    ~StereoCalibrationPipeline();
    StereoCalibrationPipeline(const StereoCalibrationPipeline&) = delete;
    StereoCalibrationPipeline(const StereoCalibrationPipeline&&) = delete;
    StereoCalibrationPipeline& operator=(const StereoCalibrationPipeline&) = delete;
    StereoCalibrationPipeline& operator=(const StereoCalibrationPipeline&&) = delete;

    void start();
    void stop();

private:
    const std::string m_leftStreamId;
    const std::string m_rightStreamId;

    std::shared_ptr<stereo::ZmqCameraSourceFilter> m_stereoVideoSource;
    std::shared_ptr<stereo::SyncMux> m_syncMux;
    std::shared_ptr<stereo::CalibrationFileSinkFilter> m_calibrationFileSinkFilter;

    filter_graph::ThreadPool<filter_graph::SourceFilterThreadRunner> m_sourceFilterThreadPool;
    filter_graph::ThreadPool<filter_graph::FilterThreadRunner> m_filterThreadPool;

    void wireGraph();
};
}
