#include "packages/stereo/include/stereo_filter.h"
#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/filter_graph/include/aggregate_sample.h"
#include "packages/utilities/opencv/datatype_conversions.h"

#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/util/json_util.h"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgproc.hpp"

#include <fstream>

namespace stereo {

constexpr float MAX_Z = 10000; // Max Z opencv supports

StereoFilter::StereoFilter(const std::string& leftStreamId, const std::string& rightStreamId, const std::string& depthStreamId,
    const std::string& systemCalibrationFile, const bool& outputPointCloud)
    : filter_graph::TransformFilter("StereoFilter", 10, 10)
    , m_leftStreamId(leftStreamId)
    , m_rightStreamId(rightStreamId)
    , m_depthStreamId(depthStreamId)
    , m_outputPointCloud(outputPointCloud)
    , m_scale(0.25f)
    , m_frameCounter(0) {

    calibration::SystemCalibration systemCalibration;

    std::ifstream file(systemCalibrationFile);
    std::stringstream buffer;
    buffer << file.rdbuf();
    if (buffer.str().size() == 0) {
        LOG(ERROR) << "Empty system calibration file";
        throw std::runtime_error("Empty system calibration file");
    }
    google::protobuf::util::JsonStringToMessage(buffer.str(), &systemCalibration);

    for (int i = 0; i < systemCalibration.cameraintrinsiccalibration().size(); i++) {
        if (systemCalibration.cameraintrinsiccalibration(i).cameraundercalibration().name() == m_leftStreamId) {
            OpenCVUtility::cameraIntrinsicProtoToOcvMat(
                systemCalibration.cameraintrinsiccalibration(i), m_leftCameraMatrix, m_leftCameraDistortion);
        } else if (systemCalibration.cameraintrinsiccalibration(i).cameraundercalibration().name() == m_rightStreamId) {
            OpenCVUtility::cameraIntrinsicProtoToOcvMat(
                systemCalibration.cameraintrinsiccalibration(i), m_rightCameraMatrix, m_rightCameraDistortion);
        }
    }
    if (m_leftCameraMatrix.empty()) {
        LOG(ERROR) << "No intrinsics found for left camera. id:" << m_leftStreamId;
        throw std::runtime_error("No intrinsics found for left camera");
    }
    if (m_leftCameraMatrix.empty()) {
        LOG(ERROR) << "No intrinsics found for right camera";
        throw std::runtime_error("No intrinsics found for right camera");
    }

    for (int i = 0; i < systemCalibration.devicetodevicecoordinatetransformation().size(); i++) {
        if (systemCalibration.devicetodevicecoordinatetransformation(i).sourcecoordinateframe().device().name() == m_leftStreamId
            && systemCalibration.devicetodevicecoordinatetransformation(i).targetcoordinateframe().device().name() == m_rightStreamId) {
            OpenCVUtility::coordinateTransformationProtoToOcvMat(
                systemCalibration.devicetodevicecoordinatetransformation(i), m_extrinsicRotationL2R, m_extrinsicTranslationL2R);
        } else if (systemCalibration.devicetodevicecoordinatetransformation(i).sourcecoordinateframe().device().name() == m_rightStreamId
            && systemCalibration.devicetodevicecoordinatetransformation(i).targetcoordinateframe().device().name() == m_leftStreamId) {
            OpenCVUtility::coordinateTransformationProtoToOcvMat(
                systemCalibration.devicetodevicecoordinatetransformation(i), m_extrinsicRotationL2R, m_extrinsicTranslationL2R);
            cv::transpose(m_extrinsicRotationL2R, m_extrinsicRotationL2R);
            m_extrinsicTranslationL2R = -m_extrinsicRotationL2R * m_extrinsicTranslationL2R;
        }
    }
    if (m_extrinsicRotationL2R.empty()) {
        LOG(ERROR) << "No coordinate transformation from the left to right camera";
        throw std::runtime_error("No coordinate transformation from the left to right camera");
    }
    LOG(INFO) << m_leftCameraMatrix;
    LOG(INFO) << m_extrinsicTranslationL2R;
    LOG(INFO) << "Created stereo filter";
}

StereoFilter::~StereoFilter() {}

void StereoFilter::receive(std::shared_ptr<filter_graph::Container> container) {
    auto leftSample = container->get(m_leftStreamId);
    auto rightSample = container->get(m_rightStreamId);
    if (leftSample.get() && rightSample.get()) {
        LOG(INFO) << "Received stereo image pair";

        auto leftImage = static_cast<filter_graph::AggregateSample<hal::CameraSample>*>(leftSample.get());
        auto rightImage = static_cast<filter_graph::AggregateSample<hal::CameraSample>*>(rightSample.get());

        auto depthSample = std::make_shared<filter_graph::AggregateSample<hal::CameraSample> >(m_depthStreamId);

        if (stereoMatch(depthSample->data(), leftImage->data(), rightImage->data())) {
            container->add(m_depthStreamId, depthSample);
        }
    }
    getOutputQueue()->enqueue(container);
    send();
}

bool StereoFilter::stereoMatch(hal::CameraSample& rangeImage, const hal::CameraSample& leftImage, const hal::CameraSample& rightImage) {

    cv::Mat leftOcvImage;
    cv::Mat rightOcvImage;

    OpenCVUtility::cameraSampleToOcvMat(leftImage, leftOcvImage, false);
    OpenCVUtility::cameraSampleToOcvMat(rightImage, rightOcvImage, false);

    cv::Ptr<cv::StereoSGBM> sgbm = cv::StereoSGBM::create(0, 16, 3);

    cv::Mat temp1, temp2;
    int method = m_scale < 1 ? cv::INTER_AREA : cv::INTER_CUBIC;
    cv::resize(rightOcvImage, temp1, cv::Size(), m_scale, m_scale, method);
    rightOcvImage = temp1;
    cv::resize(leftOcvImage, temp2, cv::Size(), m_scale, m_scale, method);
    leftOcvImage = temp2;

    m_rightCameraMatrix *= m_scale;
    m_leftCameraMatrix *= m_scale;

    if (m_frameCounter == 0) {
        cv::Size img_size = leftOcvImage.size();
        cv::Mat R1, R2, P1, P2;

        cv::fisheye::stereoRectify(m_leftCameraMatrix, m_leftCameraDistortion, m_rightCameraMatrix, m_rightCameraDistortion, img_size,
            m_extrinsicRotationL2R, m_extrinsicTranslationL2R, R1, R2, P1, P2, m_Q, cv::CALIB_ZERO_DISPARITY, img_size, 0, 1);

        cv::fisheye::initUndistortRectifyMap(m_leftCameraMatrix, m_leftCameraDistortion, R1, P1, img_size, CV_16SC2, m_map11, m_map12);
        cv::fisheye::initUndistortRectifyMap(m_rightCameraMatrix, m_rightCameraDistortion, R2, P2, img_size, CV_16SC2, m_map21, m_map22);

        m_transformToLeftCamera = R1.inv();
        m_transformToLeftCamera.convertTo(m_transformToLeftCamera, CV_32FC1);
    }

    cv::Mat img1r, img2r;
    cv::remap(leftOcvImage, img1r, m_map11, m_map12, cv::INTER_LINEAR);
    cv::remap(rightOcvImage, img2r, m_map21, m_map22, cv::INTER_LINEAR);

    leftOcvImage = img1r;
    rightOcvImage = img2r;

    int numDisparities = 128;
    sgbm->setPreFilterCap(63);
    int sgbmWinSize = 7;
    sgbm->setBlockSize(sgbmWinSize);
    int cn = rightOcvImage.channels();
    sgbm->setP1(8 * cn * sgbmWinSize * sgbmWinSize);
    sgbm->setP2(32 * cn * sgbmWinSize * sgbmWinSize);
    sgbm->setMinDisparity(0);
    sgbm->setNumDisparities(numDisparities);
    sgbm->setUniquenessRatio(10);
    sgbm->setSpeckleWindowSize(100);
    sgbm->setSpeckleRange(32);
    sgbm->setDisp12MaxDiff(1);
    sgbm->setMode(cv::StereoSGBM::MODE_SGBM);

    cv::Mat disp, dispImg;
    sgbm->compute(leftOcvImage, rightOcvImage, disp);

    disp.convertTo(dispImg, CV_8U, 255 / (numDisparities * 16.));

    cv::Mat xyz;
    /// Account for the fixed point representation used by Opencv for disparity map by dividing by 16.
    cv::reprojectImageTo3D(disp / 16.0, xyz, m_Q, true);

    rangeImage.mutable_device()->set_name("depth");
    rangeImage.mutable_device()->set_serialnumber(leftImage.device().serialnumber());
    rangeImage.mutable_systemtimestamp()->CopyFrom(leftImage.systemtimestamp());
    rangeImage.mutable_hardwaretimestamp()->CopyFrom(leftImage.hardwaretimestamp());
    rangeImage.set_id(m_frameCounter);
    m_frameCounter++;

    // ToDo: Convert range image to left camera camera coordinates
    /// Range image is in OpenCV's rectified camera coordinates
    if (!m_outputPointCloud) {
        cv::Mat channels[3];
        cv::split(xyz, channels);
        OpenCVUtility::ocvMatToCameraSample(channels[2], rangeImage);
        return true;
    }

    cv::Vec3f* point = (cv::Vec3f*)xyz.data;
    float* transformData = (float*)m_transformToLeftCamera.data;
    for (int i = 0; i < xyz.rows; i++) {
        for (int j = 0; j < xyz.cols; j++) {
            /// Remove points with minimal disparity
            if (std::abs((*point)[2] - MAX_Z) < std::numeric_limits<float>::epsilon() || std::abs((*point)[2] > MAX_Z) || (*point)[2] < 0) {
                (*point)[0] = std::numeric_limits<float>::quiet_NaN();
                (*point)[1] = std::numeric_limits<float>::quiet_NaN();
                (*point)[2] = std::numeric_limits<float>::quiet_NaN();
            }
            /// Convert 3D points from the rectified coordinates back to the left camera coordinates
            (*point)[0] = transformData[0] * (*point)[0] + transformData[1] * (*point)[1] + transformData[2] * (*point)[2];
            (*point)[1] = transformData[3] * (*point)[0] + transformData[4] * (*point)[1] + transformData[5] * (*point)[2];
            (*point)[2] = transformData[6] * (*point)[0] + transformData[7] * (*point)[1] + transformData[8] * (*point)[2];
            point++;
        }
    }
    OpenCVUtility::ocvMatToCameraSample(xyz, rangeImage);

    return true;
}
}
