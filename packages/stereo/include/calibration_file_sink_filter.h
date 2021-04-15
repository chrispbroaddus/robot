#pragma once

#include "packages/core/include/image_view.h"
#include "packages/filter_graph/include/sink_filter.h"
#include "packages/hal/proto/camera_sample.pb.h"

#include "opencv2/core.hpp"

namespace stereo {

/// Saves captured stereo images to appropriately named files for consumption by vicalib
class CalibrationFileSinkFilter : public filter_graph::SinkFilter {
public:
    CalibrationFileSinkFilter(const std::string& leftStreamId, const std::string& rightStreamId, const std::string& outputFolder);
    ~CalibrationFileSinkFilter();
    CalibrationFileSinkFilter(const CalibrationFileSinkFilter&) = delete;
    CalibrationFileSinkFilter(const CalibrationFileSinkFilter&&) = delete;
    CalibrationFileSinkFilter& operator=(const CalibrationFileSinkFilter&) = delete;
    CalibrationFileSinkFilter& operator=(const CalibrationFileSinkFilter&&) = delete;

    /// Receive captured stereo images through the filter graph
    void receive(std::shared_ptr<filter_graph::Container> container) override;

private:
    const std::string m_leftStreamId;
    const std::string m_rightStreamId;
    const std::string m_outputFolder;
    uint32_t m_frameCounter;
};
}
