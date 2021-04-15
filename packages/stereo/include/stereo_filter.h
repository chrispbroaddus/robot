#include "packages/core/include/image_view.h"
#include "packages/filter_graph/include/transform_filter.h"
#include "packages/hal/proto/camera_sample.pb.h"

#include "opencv2/core.hpp"

namespace stereo {

class StereoFilter : public filter_graph::TransformFilter {
public:
    StereoFilter(const std::string& leftStreamId, const std::string& rightStreamId, const std::string& depthStreamId,
        const std::string& systemCalibrationFile, const bool& outputPointCloud);
    ~StereoFilter();
    StereoFilter(const StereoFilter&) = delete;
    StereoFilter(const StereoFilter&&) = delete;
    StereoFilter& operator=(const StereoFilter&) = delete;
    StereoFilter& operator=(const StereoFilter&&) = delete;

    void receive(std::shared_ptr<filter_graph::Container> container) override;

private:
    const std::string m_leftStreamId;
    const std::string m_rightStreamId;
    const std::string m_depthStreamId;
    const bool m_outputPointCloud;
    const float m_scale;
    uint32_t m_frameCounter;
    cv::Mat m_leftCameraMatrix;
    cv::Mat m_leftCameraDistortion;
    cv::Mat m_rightCameraMatrix;
    cv::Mat m_rightCameraDistortion;
    cv::Mat m_extrinsicRotationL2R;
    cv::Mat m_extrinsicTranslationL2R;
    cv::Mat m_map11;
    cv::Mat m_map12;
    cv::Mat m_map21;
    cv::Mat m_map22;
    cv::Mat m_Q;
    cv::Mat m_transformToLeftCamera;

    bool stereoMatch(hal::CameraSample& rangeImage, const hal::CameraSample& leftImage, const hal::CameraSample& rightImage);
};
}
