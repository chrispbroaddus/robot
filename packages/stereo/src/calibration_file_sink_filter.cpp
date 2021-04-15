#include "packages/stereo/include/calibration_file_sink_filter.h"
#include "packages/filter_graph/include/aggregate_sample.h"
#include "packages/utilities/opencv/datatype_conversions.h"

#include "glog/logging.h"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <fstream>

namespace stereo {

CalibrationFileSinkFilter::CalibrationFileSinkFilter(
    const std::string& leftStreamId, const std::string& rightStreamId, const std::string& outputFolder)
    : filter_graph::SinkFilter("CalibrationFileSinkFilter", 10)
    , m_leftStreamId(leftStreamId)
    , m_rightStreamId(rightStreamId)
    , m_outputFolder(outputFolder)
    , m_frameCounter(0) {}

CalibrationFileSinkFilter::~CalibrationFileSinkFilter() {}

void CalibrationFileSinkFilter::receive(std::shared_ptr<filter_graph::Container> container) {
    auto leftSample = container->get(m_leftStreamId);
    auto rightSample = container->get(m_rightStreamId);
    if (leftSample.get() && rightSample.get()) {
        LOG(INFO) << "Received stereo image pair";

        auto leftImage = static_cast<filter_graph::AggregateSample<hal::CameraSample>*>(leftSample.get());
        auto rightImage = static_cast<filter_graph::AggregateSample<hal::CameraSample>*>(rightSample.get());

        cv::Mat leftOcvImage;
        cv::Mat rightOcvImage;
        OpenCVUtility::cameraSampleToOcvMat(leftImage->data(), leftOcvImage, true);
        OpenCVUtility::cameraSampleToOcvMat(rightImage->data(), rightOcvImage, true);
        if (leftOcvImage.channels() != 1) {
            cv::cvtColor(leftOcvImage, leftOcvImage, CV_BGR2GRAY);
        }
        cv::imwrite(m_outputFolder + "/" + "left_" + std::to_string(m_frameCounter) + ".pgm", leftOcvImage);
        if (rightOcvImage.channels() != 1) {
            cv::cvtColor(rightOcvImage, rightOcvImage, CV_BGR2GRAY);
        }
        cv::imwrite(m_outputFolder + "/" + "right_" + std::to_string(m_frameCounter++) + ".pgm", rightOcvImage);
    }
}
}
