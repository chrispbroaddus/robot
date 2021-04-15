#include "packages/calibration/include/kannala_brandt_distortion_model.h"
#include "packages/calibration/include/kb4_image_undistortion.h"
#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/feature_detectors/include/feature_point_selector.h"
#include "packages/feature_detectors/include/harris_feature_detector.h"
#include "packages/feature_tracker/include/ncc_feature_matcher.h"
#include "packages/feature_tracker/include/stereo_feature_matcher.h"
#include "packages/feature_tracker/include/stereo_feature_track_index.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_select.h"
#include "packages/utilities/opencv/datatype_conversions.h"

#include "SDL.h"
#include "gflags/gflags.h"
#include "google/protobuf/util/json_util.h"
#include <fstream>
#include <thread>

DEFINE_string(leftCameraAddress, "tcp://localhost:5350", "left camera address");
DEFINE_string(leftTopic, "camera0", "left camera address");
DEFINE_string(rightCameraAddress, "tcp://localhost:5351", "right camera address");
DEFINE_string(rightTopic, "camera1", "right camera address");
DEFINE_string(systemCalibrationFile, "", "system calibration file");
DEFINE_double(newFocalLengthScale, 0.6f, "scale factor for new focal length used for undistortion");

typedef core::DynamicStorage<core::rgb8_scalar_t, core::AlignedMemoryAllocator> rgb8_aligned_storage_t;

static bool s_stopViewer = false;

class FeatureTracker {
public:
    static constexpr float kHarrisConstant = 0.06;
    static constexpr size_t kHarrisWindowSize = 11;
    static constexpr float kSearchRadiusPercentage = 0.1;
    static constexpr float kMinNccScore = 0.07;

    using NccFeatureStore = feature_tracker::NccFeatureStore<kHarrisWindowSize>;
    using NccFeatureMatcher = feature_tracker::NccFeatureMatcher<kHarrisWindowSize>;

    FeatureTracker(const size_t maxPoints, const size_t maxTrackFrameBuffer)
        : m_maxPoints(maxPoints)
        , m_detector(kHarrisConstant)
        , m_trackIndex(maxTrackFrameBuffer) {}
    ~FeatureTracker() = default;

    /// Track feature from previous frame to current frame
    /// \param image The image to process
    void nextFrame(const core::ImageView<core::ImageType::uint8>& image) {
        std::swap(m_currFeatureStore, m_prevFeatureStore);

        if (!m_selector.get()) {
            m_selector.reset(
                new feature_detectors::FeaturePointSelector(image.rows, image.cols, numBucketsRows, numBucketsCols, m_maxPoints));
        }
        if (!m_matcher.get()) {
            m_matcher.reset(new NccFeatureMatcher(image.cols * kSearchRadiusPercentage, image.rows, image.cols, kMinNccScore));
        }

        const std::vector<feature_detectors::FeaturePoint>& points = m_detector.detect(image);
        const std::vector<feature_detectors::FeaturePoint>& selectedPoints = m_selector->select(points);
        m_currFeatureStore = std::make_shared<NccFeatureStore>(image, selectedPoints);
        m_matcher->updateCurrentFrame(m_currFeatureStore.get());
        m_trackIds = m_trackIndex.addMatches(selectedPoints, m_matcher->matchFeatures());
    }

    /// \return Track index
    const feature_tracker::details::stereo_feature_track_index_type& getTrackIndex() const { return m_trackIndex; }

    /// \return Feature store of current frame
    const NccFeatureStore& getFeatureStore() const { return *m_currFeatureStore.get(); }

    /// \return Feature points of current frame
    const std::vector<feature_detectors::FeaturePoint>& getPoints() const { return m_selector->points(); }

    /// \return Track id's for each feature point in current frame
    const std::vector<feature_tracker::details::track_id>& getTrackIds() const { return m_trackIds; }

private:
    static constexpr size_t numBucketsRows = 10;
    static constexpr size_t numBucketsCols = 10;

    /// Maximum number of points per frame
    const size_t m_maxPoints;

    /// Feature detector
    feature_detectors::HarrisFeatureDetector m_detector;
    std::unique_ptr<feature_detectors::FeaturePointSelector> m_selector;
    std::shared_ptr<NccFeatureStore> m_prevFeatureStore;
    std::shared_ptr<NccFeatureStore> m_currFeatureStore;

    /// Feature matcher
    std::shared_ptr<NccFeatureMatcher> m_matcher;

    /// Track index
    feature_tracker::details::stereo_feature_track_index_type m_trackIndex;
    std::vector<feature_tracker::details::track_id> m_trackIds;
};

class StereoFeatureTracker {
public:
    static constexpr size_t kHarrisWindowSize = 11;
    static constexpr float kMinNccScore = 0.6;
    static constexpr float kMaxPointDistance = 5;

    using StereoFeatureMatcher = feature_tracker::StereoFeatureMatcher<kHarrisWindowSize, float>;

    StereoFeatureTracker(const size_t maxPoints, const size_t maxTrackFrameBuffer,
        const calibration::CameraIntrinsicCalibration& leftIntrinsics, const calibration::CameraIntrinsicCalibration rightIntrinsics,
        const Eigen::Matrix<float, 3, 3>& rotationMatrix, const Eigen::Matrix<float, 3, 1>& translationVector)
        : m_leftFeatureTracker(maxPoints, maxTrackFrameBuffer)
        , m_rightFeatureTracker(maxPoints, maxTrackFrameBuffer)
        , m_stereFeatureMatcher(kMaxPointDistance, kMinNccScore, leftIntrinsics, rightIntrinsics, rotationMatrix, translationVector) {}
    ~StereoFeatureTracker() = default;

    /// Process the next frame
    /// \param leftImage Left image to process
    /// \param rightImage Right image to process
    void nextFrame(const core::ImageView<core::ImageType::uint8>& leftImage, const core::ImageView<core::ImageType::uint8>& rightImage) {
        m_leftFeatureTracker.nextFrame(leftImage);
        m_rightFeatureTracker.nextFrame(rightImage);
        m_stereoMatches
            = m_stereFeatureMatcher.matchFeatureStores(m_leftFeatureTracker.getFeatureStore(), m_rightFeatureTracker.getFeatureStore());
    }

    /// \return Track index for the left camera
    const feature_tracker::details::stereo_feature_track_index_type& getLeftTrackIndex() const {
        return m_leftFeatureTracker.getTrackIndex();
    }

    /// \return Track index for the right camera
    const feature_tracker::details::stereo_feature_track_index_type& getRightTrackIndex() const {
        return m_rightFeatureTracker.getTrackIndex();
    }

    /// \return Matches from left to right of current image
    const std::vector<std::pair<int, int> >& stereoMatches() const { return m_stereoMatches; }

    /// \return Left feature tracker
    const FeatureTracker& getLeftFeatureTracker() const { return m_leftFeatureTracker; }

    /// \return Right feature tracker
    const FeatureTracker& getRightFeatureTracker() const { return m_rightFeatureTracker; }

private:
    /// Left camera feature tracker
    FeatureTracker m_leftFeatureTracker;

    /// Right camera feature tracker
    FeatureTracker m_rightFeatureTracker;

    /// Stereo matcher
    StereoFeatureMatcher m_stereFeatureMatcher;
    std::vector<std::pair<int, int> > m_stereoMatches;
};

template <typename T> class StereoFeatureTrackerVisualization {
public:
    StereoFeatureTrackerVisualization(const calibration::CameraIntrinsicCalibration& leftIntrinsics,
        const calibration::CameraIntrinsicCalibration& rightIntrinsics, const float newFocalLengthScale)
        : m_window(0)
        , m_renderer(0)
        , m_texture(0) {

        m_leftK << leftIntrinsics.scaledfocallengthx(), 0, leftIntrinsics.opticalcenterx(), 0, leftIntrinsics.scaledfocallengthy(),
            leftIntrinsics.opticalcentery(), 0, 0, 1;
        m_leftCoef << leftIntrinsics.kannalabrandt().radialdistortioncoefficientk(0),
            leftIntrinsics.kannalabrandt().radialdistortioncoefficientk(1), leftIntrinsics.kannalabrandt().radialdistortioncoefficientk(2),
            leftIntrinsics.kannalabrandt().radialdistortioncoefficientk(3);
        m_rightK << rightIntrinsics.scaledfocallengthx(), 0, rightIntrinsics.opticalcenterx(), 0, rightIntrinsics.scaledfocallengthy(),
            rightIntrinsics.opticalcentery(), 0, 0, 1;
        m_rightCoef << rightIntrinsics.kannalabrandt().radialdistortioncoefficientk(0),
            rightIntrinsics.kannalabrandt().radialdistortioncoefficientk(1),
            rightIntrinsics.kannalabrandt().radialdistortioncoefficientk(2),
            rightIntrinsics.kannalabrandt().radialdistortioncoefficientk(3);
        m_leftCameraModel.reset(new calibration::KannalaBrandtRadialDistortionModel4<T>(m_leftK, m_leftCoef, 10, 0));
        m_rightCameraModel.reset(new calibration::KannalaBrandtRadialDistortionModel4<T>(m_rightK, m_rightCoef, 10, 0));
        float newFocalLength = newFocalLengthScale * m_leftK(0, 0);
        uint32_t outputRows = 2048;
        uint32_t outputCols = 2448;
        m_newK << newFocalLength, 0, outputCols / 2, 0, newFocalLength, outputRows / 2, 0, 0, 1;
        m_linearCameraModel.reset(new calibration::LinearCameraModel<T>(m_newK));
        m_leftImageUndistortion.reset(new calibration::Kb4ImageUndistortion<T>(m_leftK, m_leftCoef, 2048, 2448, m_newK));
        m_rightImageUndistortion.reset(new calibration::Kb4ImageUndistortion<T>(m_rightK, m_rightCoef, 2048, 2448, m_newK));
    }
    ~StereoFeatureTrackerVisualization() {
        SDL_DestroyWindow(m_window);
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyTexture(m_texture);
    }

    void copyStereoImagesToTexture(const hal::CameraSample& leftDistortedImage, const hal::CameraSample& rightDistortedImage) {

        hal::Image leftImage;
        hal::Image rightImage;
        m_leftImageUndistortion->undistortImage(leftDistortedImage.image(), leftImage);
        m_rightImageUndistortion->undistortImage(rightDistortedImage.image(), rightImage);

        core::DynamicImage<core::ImageType::rgb8, rgb8_aligned_storage_t> image(leftImage.rows(), leftImage.cols() * 2);
        auto view = image.view();

        for (size_t row = 0; row < leftImage.rows(); row++) {
            core::rgb8_pixel* leftStereoRowPixels = (core::rgb8_pixel*)&view.data[row * view.stride];
            core::rgb8_pixel* rightStereoRowPixels = (core::rgb8_pixel*)&view.data[row * view.stride + leftImage.cols() * 3];
            for (size_t col = 0; col < leftImage.cols(); col++) {
                // Left image
                leftStereoRowPixels[col].r = leftImage.data()[row * leftImage.stride() + col];
                leftStereoRowPixels[col].g = leftStereoRowPixels[col].r;
                leftStereoRowPixels[col].b = leftStereoRowPixels[col].r;
                // Right image
                rightStereoRowPixels[col].r = rightImage.data()[row * rightImage.stride() + col];
                rightStereoRowPixels[col].g = rightStereoRowPixels[col].r;
                rightStereoRowPixels[col].b = rightStereoRowPixels[col].r;
            }
        }

        CHECK(SDL_UpdateTexture(m_texture, NULL, view.data, view.stride) == 0);
    }

    void draw(const hal::CameraSample& leftImage, const hal::CameraSample& rightImage) {
        if (!m_window) {
            m_imageRows = leftImage.image().rows();
            m_imageCols = leftImage.image().cols();

            const size_t cols = leftImage.image().cols() + rightImage.image().cols();
            const size_t rows = leftImage.image().rows();
            m_window = SDL_CreateWindow("stereo video", cols, rows, cols, rows, SDL_WINDOW_RESIZABLE);
            m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);
            m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, cols, rows);
            SDL_SetRenderTarget(m_renderer, m_texture);
        }
        copyStereoImagesToTexture(leftImage, rightImage);
        CHECK(SDL_RenderCopy(m_renderer, m_texture, NULL, NULL) == 0);
    }

    void drawTracks(const StereoFeatureTracker& stereoTracker) {
        drawTracks(stereoTracker.getLeftTrackIndex(), 0);
        drawTracks(stereoTracker.getRightTrackIndex(), 1);
    }

    void drawMatches(const StereoFeatureTracker& stereoTracker) {
        const std::vector<std::pair<int, int> >& matches = stereoTracker.stereoMatches();
        const std::vector<feature_detectors::FeaturePoint>& leftPoints = stereoTracker.getLeftFeatureTracker().getPoints();
        const std::vector<feature_detectors::FeaturePoint>& rightPoints = stereoTracker.getRightFeatureTracker().getPoints();
        const std::vector<feature_tracker::details::track_id>& leftTrackIds = stereoTracker.getLeftFeatureTracker().getTrackIds();
        const std::vector<feature_tracker::details::track_id>& rightTrackIds = stereoTracker.getRightFeatureTracker().getTrackIds();

        int windowWidth;
        int windowHeight;
        SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
        float xScale = (float)windowWidth / (2.0f * m_imageCols);
        float yScale = (float)windowHeight / m_imageRows;

        for (const auto& match : matches) {
            const feature_detectors::FeaturePoint& leftPoint = leftPoints[match.first];
            const feature_detectors::FeaturePoint& rightPoint = rightPoints[match.second];

            if (stereoTracker.getLeftFeatureTracker().getTrackIndex().getTrack(leftTrackIds[match.first]).m_frames.size() >= 5
                && stereoTracker.getRightFeatureTracker().getTrackIndex().getTrack(rightTrackIds[match.second]).m_frames.size() >= 5) {
                Eigen::Matrix<T, 2, 1> leftPixel;
                Eigen::Matrix<T, 2, 1> rightPixel;
                Eigen::Matrix<T, 3, 1> ray;
                leftPixel(0) = leftPoint.x();
                leftPixel(1) = leftPoint.y();
                ray = m_leftCameraModel->unproject(leftPixel);
                leftPixel = m_linearCameraModel->project(ray);
                rightPixel(0) = rightPoint.x();
                rightPixel(1) = rightPoint.y();
                ray = m_rightCameraModel->unproject(rightPixel);
                rightPixel = m_linearCameraModel->project(ray);
                SDL_RenderDrawLine(m_renderer, (int)(leftPixel(0) * xScale), (int)(leftPixel(1) * yScale),
                    (int)((m_imageCols + rightPixel(0)) * xScale), (int)(rightPixel(1) * yScale));
            }
        }
    }

    void present() {
        SDL_RenderPresent(m_renderer);
        SDL_RenderClear(m_renderer);
        SDL_PollEvent(nullptr);
    }

private:
    // Number of rows of image
    int m_imageRows;

    // Number of cols of image
    int m_imageCols;

    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    SDL_Texture* m_texture;

    Eigen::Matrix<T, 3, 3> m_leftK;
    Eigen::Matrix<T, 4, 1> m_leftCoef;
    Eigen::Matrix<T, 3, 3> m_rightK;
    Eigen::Matrix<T, 4, 1> m_rightCoef;
    Eigen::Matrix<T, 3, 3> m_newK;
    std::unique_ptr<calibration::Kb4ImageUndistortion<T> > m_leftImageUndistortion;
    std::unique_ptr<calibration::Kb4ImageUndistortion<T> > m_rightImageUndistortion;
    std::unique_ptr<calibration::KannalaBrandtRadialDistortionModel4<T> > m_leftCameraModel;
    std::unique_ptr<calibration::KannalaBrandtRadialDistortionModel4<T> > m_rightCameraModel;
    std::unique_ptr<calibration::LinearCameraModel<T> > m_linearCameraModel;

    void drawTracks(const feature_tracker::details::stereo_feature_track_index_type& trackIndex, const size_t imageIndex) {

        const std::vector<feature_tracker::details::stereo_feature_track_index_type::track_id> trackIds
            = trackIndex.getTracksOfMinLength(5);

        const size_t currentFrameId = trackIndex.getNewestFrameId();

        int windowWidth;
        int windowHeight;
        SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
        float xScale = (float)windowWidth / (2.0f * m_imageCols);
        float yScale = (float)windowHeight / m_imageRows;

        for (const auto& trackId : trackIds) {
            const feature_tracker::details::track_type& track = trackIndex.getTrack(trackId);
            if (track.m_frames.find(currentFrameId) != track.m_frames.end()) {
                const feature_tracker::details::frame_type& frame = trackIndex.getFrame(currentFrameId);
                const int x = (int)frame.m_points.find(trackId)->second.m_pixelPoint.x();
                const int y = (int)frame.m_points.find(trackId)->second.m_pixelPoint.y();
                Eigen::Matrix<T, 2, 1> pixel;
                Eigen::Matrix<T, 3, 1> ray;
                pixel(0) = x;
                pixel(1) = y;
                if (imageIndex == 0) {
                    ray = m_leftCameraModel->unproject(pixel);
                    pixel = m_linearCameraModel->project(ray);
                } else {
                    ray = m_rightCameraModel->unproject(pixel);
                    pixel = m_linearCameraModel->project(ray);
                }

                constexpr int rectSize = 6;
                SDL_Rect rect;
                rect.x = (int)std::round((pixel(0) - rectSize / 2.0f + m_imageCols * imageIndex) * xScale);
                rect.y = (int)std::round((pixel(1) - rectSize / 2.0f) * yScale);
                rect.w = rectSize;
                rect.h = rectSize;
                SDL_RenderDrawRect(m_renderer, &rect);
            }
        }
    }
};

class StereoTrackingDemoApp {
public:
    static constexpr size_t kMaxPoints = 500;
    static constexpr size_t kMaxTrackFrameBuffer = 5;

    StereoTrackingDemoApp(const std::string& leftCameraServerAddress, const std::string& rightCameraServerAddress,
        const std::string& leftTopic, const std::string& rightTopic, const calibration::CameraIntrinsicCalibration& leftIntrinsics,
        const calibration::CameraIntrinsicCalibration& rightIntrinsics, const Eigen::Matrix<float, 3, 3>& rotationMatrix,
        const Eigen::Matrix<float, 3, 1>& translationVector, const float newFocalLengthScale)
        : m_context(2)
        , m_leftCameraSocket(m_context, ZMQ_SUB)
        , m_rightCameraSocket(m_context, ZMQ_SUB)
        , m_stereoTracker(kMaxPoints, kMaxTrackFrameBuffer, leftIntrinsics, rightIntrinsics, rotationMatrix, translationVector)
        , m_visualizer(leftIntrinsics, rightIntrinsics, newFocalLengthScale) {
        const std::string topicleft = leftTopic;
        const std::string topicRight = rightTopic;

        m_leftCameraSocket.setsockopt(ZMQ_RCVHWM, 2);
        m_leftCameraSocket.connect(leftCameraServerAddress);
        m_leftCameraSocket.setsockopt(ZMQ_SUBSCRIBE, topicleft.c_str(), topicleft.size());

        m_rightCameraSocket.setsockopt(ZMQ_RCVHWM, 2);
        m_rightCameraSocket.connect(rightCameraServerAddress);
        m_rightCameraSocket.setsockopt(ZMQ_SUBSCRIBE, topicRight.c_str(), topicRight.size());
    };
    ~StereoTrackingDemoApp() = default;

    void run() {
        std::list<std::shared_ptr<hal::CameraSample> > leftFrameQueue;
        std::list<std::shared_ptr<hal::CameraSample> > rightFrameQueue;

        m_selectLoop.OnProtobuf<hal::CameraSample>(m_leftCameraSocket, "camera0", [&](const hal::CameraSample& sample) {
            auto ptr = std::make_shared<hal::CameraSample>(sample);
            leftFrameQueue.push_back(ptr);
        });
        m_selectLoop.OnProtobuf<hal::CameraSample>(m_rightCameraSocket, "camera1", [&](const hal::CameraSample& sample) {
            auto ptr = std::make_shared<hal::CameraSample>(sample);
            rightFrameQueue.push_back(ptr);
        });

        while (m_selectLoop.Poll() && !s_stopViewer) {

            if (leftFrameQueue.size() == 0 || rightFrameQueue.size() == 0) {
                continue;
            }

            std::shared_ptr<hal::CameraSample> leftImage;
            while (leftFrameQueue.size()) {
                leftImage = leftFrameQueue.back();
                leftFrameQueue.pop_back();
            }

            std::shared_ptr<hal::CameraSample> rightImage;
            while (rightFrameQueue.size()) {
                rightImage = rightFrameQueue.back();
                rightFrameQueue.pop_back();
            }

            core::ImageView<core::ImageType::uint8> leftView(leftImage->image().rows(), leftImage->image().cols(),
                leftImage->image().stride(), (unsigned char*)leftImage->image().data().c_str());
            core::ImageView<core::ImageType::uint8> rightView(rightImage->image().rows(), rightImage->image().cols(),
                rightImage->image().stride(), (unsigned char*)rightImage->image().data().c_str());

            m_stereoTracker.nextFrame(leftView, rightView);

            m_visualizer.draw(*leftImage.get(), *rightImage.get());
            m_visualizer.drawTracks(m_stereoTracker);
            m_visualizer.drawMatches(m_stereoTracker);
            m_visualizer.present();
        }
    }

    zmq::context_t m_context;
    zmq::socket_t m_leftCameraSocket;
    zmq::socket_t m_rightCameraSocket;
    net::ZMQSelectLoop m_selectLoop;
    StereoFeatureTracker m_stereoTracker;
    StereoFeatureTrackerVisualization<float> m_visualizer;
};

void SIGINTHandler(int signum) {
    LOG(INFO) << "Shutting down stereo demo app";
    s_stopViewer = true;
}

int main(int argc, char* argv[]) {
    signal(SIGINT, SIGINTHandler);

    gflags::SetUsageMessage("Stereo tracking demo");
    gflags::SetVersionString("1.0.0");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    if (FLAGS_systemCalibrationFile.empty()) {
        gflags::ShowUsageWithFlagsRestrict(argv[0], "applications/stereo_tracking_demo/stereo_tracking_demo.cpp");
        return 0;
    }

    calibration::SystemCalibration systemCalibration;
    calibration::CameraIntrinsicCalibration leftCameraIntrinsics;
    calibration::CameraIntrinsicCalibration rightCameraIntrinsics;

    std::ifstream file(FLAGS_systemCalibrationFile);
    std::stringstream buffer;
    buffer << file.rdbuf();
    if (buffer.str().size() == 0) {
        LOG(ERROR) << "Empty system calibration file";
        throw std::runtime_error("Empty system calibration file");
    }
    google::protobuf::util::JsonStringToMessage(buffer.str(), &systemCalibration);

    for (int i = 0; i < systemCalibration.cameraintrinsiccalibration().size(); i++) {
        if (systemCalibration.cameraintrinsiccalibration(i).cameraundercalibration().name() == FLAGS_leftTopic) {
            leftCameraIntrinsics = systemCalibration.cameraintrinsiccalibration(i);
        } else if (systemCalibration.cameraintrinsiccalibration(i).cameraundercalibration().name() == FLAGS_rightTopic) {
            rightCameraIntrinsics = systemCalibration.cameraintrinsiccalibration(i);
        }
    }
    if (leftCameraIntrinsics.DebugString() == "") {
        LOG(ERROR) << "No intrinsics found for left camera with name: " << FLAGS_leftTopic;
        throw std::runtime_error("No intrinsics found for left camera");
    }
    if (rightCameraIntrinsics.DebugString() == "") {
        LOG(ERROR) << "No intrinsics found for right camera with name: " << FLAGS_rightTopic;
        throw std::runtime_error("No intrinsics found for right camera");
    }

    cv::Mat ocvRotationMatrix;
    cv::Mat ocvTranslationVector;
    Eigen::Matrix<float, 3, 3> rotationMatrix;
    Eigen::Matrix<float, 3, 1> translationVector;
    for (int i = 0; i < systemCalibration.devicetodevicecoordinatetransformation().size(); i++) {
        if (systemCalibration.devicetodevicecoordinatetransformation(i).sourcecoordinateframe().device().name() == FLAGS_rightTopic
            && systemCalibration.devicetodevicecoordinatetransformation(i).targetcoordinateframe().device().name() == FLAGS_leftTopic) {
            OpenCVUtility::coordinateTransformationProtoToOcvMat(
                systemCalibration.devicetodevicecoordinatetransformation(i), ocvRotationMatrix, ocvTranslationVector);
        } else if (systemCalibration.devicetodevicecoordinatetransformation(i).sourcecoordinateframe().device().name() == FLAGS_leftTopic
            && systemCalibration.devicetodevicecoordinatetransformation(i).targetcoordinateframe().device().name() == FLAGS_rightTopic) {
            OpenCVUtility::coordinateTransformationProtoToOcvMat(
                systemCalibration.devicetodevicecoordinatetransformation(i), ocvRotationMatrix, ocvTranslationVector);
            cv::transpose(ocvRotationMatrix, ocvRotationMatrix);
            ocvTranslationVector = -ocvRotationMatrix * ocvTranslationVector;
        }
    }
    rotationMatrix << ocvRotationMatrix.at<double>(0, 0), ocvRotationMatrix.at<double>(0, 1), ocvRotationMatrix.at<double>(0, 2),
        ocvRotationMatrix.at<double>(1, 0), ocvRotationMatrix.at<double>(1, 1), ocvRotationMatrix.at<double>(1, 2),
        ocvRotationMatrix.at<double>(2, 0), ocvRotationMatrix.at<double>(2, 1), ocvRotationMatrix.at<double>(2, 2);
    translationVector << ocvTranslationVector.at<double>(0), ocvTranslationVector.at<double>(1), ocvTranslationVector.at<double>(2);

    StereoTrackingDemoApp app(FLAGS_leftCameraAddress, FLAGS_rightCameraAddress, FLAGS_leftTopic, FLAGS_rightTopic, leftCameraIntrinsics,
        rightCameraIntrinsics, rotationMatrix, translationVector, (float)FLAGS_newFocalLengthScale);
    app.run();

    return 0;
}
