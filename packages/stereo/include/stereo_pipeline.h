
#pragma once

#include "packages/stereo/include/stereo_filter.h"
#include "packages/stereo/include/sync_mux_filter.h"
#include "packages/stereo/include/zmq_camera_source_filter.h"
#include "packages/stereo/include/zmq_depth_sink_filter.h"

namespace stereo {

class StereoPipeline {
public:
    StereoPipeline(const std::string& leftServerAddr, const std::string& leftTopic, const std::string& rightServerAddr,
        const std::string& rightTopic, const std::string& systemCalibrationFile, const std::string& depthPublisherAddress,
        const std::string& depthTopic, const bool& outputPointCloud);
    ~StereoPipeline();
    StereoPipeline(const StereoPipeline&) = delete;
    StereoPipeline(const StereoPipeline&&) = delete;
    StereoPipeline& operator=(const StereoPipeline&) = delete;
    StereoPipeline& operator=(const StereoPipeline&&) = delete;

    void start();
    void stop();

private:
    const std::string m_leftStreamId;
    const std::string m_rightStreamId;
    const std::string m_depthStreamId;

    std::shared_ptr<stereo::ZmqCameraSourceFilter> m_stereoVideoSource;
    std::shared_ptr<stereo::SyncMux> m_syncMux;
    std::shared_ptr<stereo::StereoFilter> m_stereoFilter;
    std::shared_ptr<stereo::ZmqDepthSinkFilter> m_depthSinkFilter;

    filter_graph::ThreadPool<filter_graph::SourceFilterThreadRunner> m_sourceFilterThreadPool;
    filter_graph::ThreadPool<filter_graph::FilterThreadRunner> m_filterThreadPool;

    void wireGraph();
};
}