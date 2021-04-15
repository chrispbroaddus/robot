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

typedef core::DynamicStorage<core::rgb8_scalar_t, core::AlignedMemoryAllocator> rgb8_aligned_storage_t;

static bool s_stopViewer = false;

template <typename T> class EpipolarLineVisualization {
public:
    EpipolarLineVisualization(const calibration::CameraIntrinsicCalibration& leftIntrinsics,
        const calibration::CameraIntrinsicCalibration& rightIntrinsics, const Eigen::Matrix<T, 3, 3>& rotationMatrix,
        const Eigen::Matrix<T, 3, 1>& translationVector)
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

        m_essentialMatrix = feature_tracker::computeEssentialMatrix(rotationMatrix, translationVector);
        m_leftCameraModel.reset(new calibration::KannalaBrandtRadialDistortionModel4<T>(m_leftK, m_leftCoef, 10, 0));
        m_rightCameraModel.reset(new calibration::KannalaBrandtRadialDistortionModel4<T>(m_rightK, m_rightCoef, 10, 0));
    }
    ~EpipolarLineVisualization() {
        SDL_DestroyWindow(m_window);
        SDL_DestroyRenderer(m_renderer);
        SDL_DestroyTexture(m_texture);
    }

    void copyStereoImagesToTexture(const hal::CameraSample& leftDistortedImage, const hal::CameraSample& rightDistortedImage) {

        hal::Image leftImage = leftDistortedImage.image();
        hal::Image rightImage = rightDistortedImage.image();

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

    void drawEpipolarLine() {

        int windowWidth;
        int windowHeight;
        SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
        float xScale = (2.0f * m_imageCols) / (float)windowWidth;
        float yScale = (float)m_imageRows / windowHeight;

        int x;
        int y;
        SDL_GetMouseState(&x, &y);

        constexpr int rectSize = 6;
        SDL_Rect rect;
        rect.x = (int)std::round(x - rectSize / 2.0f);
        rect.y = (int)std::round(y - rectSize / 2.0f);
        rect.w = rectSize;
        rect.h = rectSize;
        SDL_RenderDrawRect(m_renderer, &rect);

        Eigen::Matrix<T, 2, 1> pixel;
        Eigen::Matrix<T, 3, 1> epipolarLine;
        Eigen::Matrix<T, 2, 1> linePoint0;
        Eigen::Matrix<T, 2, 1> linePoint1;
        Eigen::Matrix<T, 3, 1> ray;

        constexpr T maximumDistanceToLine = 0.003;

        if (x < windowWidth / 2) {
            pixel(0) = x * xScale;
            pixel(1) = y * yScale;
            ray = m_leftCameraModel->unproject(pixel);
            epipolarLine = feature_tracker::computeRightEpipolarLine(m_essentialMatrix, ray);
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    ray(0) = -1 + 2.0f * i / 100.0f;
                    ray(1) = -1 + 2.0f * j / 100.0f;
                    ray(2) = 1;
                    if (feature_tracker::shortestDistPointToLine(ray, epipolarLine) < maximumDistanceToLine) {
                        pixel = m_rightCameraModel->project(ray);
                        pixel(0) += m_imageCols;
                        SDL_Rect rect1;
                        rect1.x = (int)std::round(pixel(0) - rectSize / 2.0f) / xScale;
                        rect1.y = (int)std::round(pixel(1) - rectSize / 2.0f) / yScale;
                        rect1.w = rectSize;
                        rect1.h = rectSize;
                        SDL_RenderDrawRect(m_renderer, &rect1);
                    }
                }
            }
            for (int i = 0; i < 50; i++) {
                if (i == 0) {
                    ray(0) = -1;
                    ray(1) = getYCoordinateForPointOnLine(epipolarLine, ray(0));
                    ray(2) = 1;
                    linePoint0 = m_rightCameraModel->project(ray);
                    linePoint0(0) += m_imageCols;
                } else {
                    linePoint0 = linePoint1;
                }
                ray(0) = -1.0f + 2.0f * i / 50.0f;
                ray(1) = getYCoordinateForPointOnLine(epipolarLine, ray(0));
                ray(2) = 1;
                linePoint1 = m_rightCameraModel->project(ray);
                linePoint1(0) += m_imageCols;
                SDL_RenderDrawLine(m_renderer, (int)(linePoint0(0) / xScale), (int)(linePoint0(1) / yScale), (int)(linePoint1(0) / xScale),
                    (int)(linePoint1(1) / yScale));
            }
        } else {
            pixel(0) = (x - windowWidth / 2.0f) * xScale;
            pixel(1) = y * yScale;
            ray = m_rightCameraModel->unproject(pixel);
            epipolarLine = feature_tracker::computeLeftEpipolarLine(m_essentialMatrix, ray);
            for (int i = 0; i < 100; i++) {
                for (int j = 0; j < 100; j++) {
                    ray(0) = -1 + 2.0f * i / 100.0f;
                    ray(1) = -1 + 2.0f * j / 100.0f;
                    ray(2) = 1;
                    if (feature_tracker::shortestDistPointToLine(ray, epipolarLine) < maximumDistanceToLine) {
                        pixel = m_leftCameraModel->project(ray);
                        SDL_Rect rect1;
                        rect1.x = (int)std::round(pixel(0) - rectSize / 2.0f) / xScale;
                        rect1.y = (int)std::round(pixel(1) - rectSize / 2.0f) / yScale;
                        rect1.w = rectSize;
                        rect1.h = rectSize;
                        SDL_RenderDrawRect(m_renderer, &rect1);
                    }
                }
            }
            for (int i = 0; i < 50; i++) {
                if (i == 0) {
                    ray(0) = -1;
                    ray(1) = getYCoordinateForPointOnLine(epipolarLine, ray(0));
                    ray(2) = 1;
                    linePoint0 = m_leftCameraModel->project(ray);
                } else {
                    linePoint0 = linePoint1;
                }
                ray(0) = -1.0f + 2.0f * i / 50.0f;
                ray(1) = getYCoordinateForPointOnLine(epipolarLine, ray(0));
                ray(2) = 1;
                linePoint1 = m_leftCameraModel->project(ray);
                SDL_RenderDrawLine(m_renderer, (int)(linePoint0(0) / xScale), (int)(linePoint0(1) / yScale), (int)(linePoint1(0) / xScale),
                    (int)(linePoint1(1) / yScale));
            }
        }
    }

    void present() {
        SDL_RenderPresent(m_renderer);
        SDL_RenderClear(m_renderer);
        SDL_PollEvent(nullptr);
    }

private:
    float getYCoordinateForPointOnLine(Eigen::Matrix<T, 3, 1> line, int x) { return (-line(2) - line(0) * x) / line(1); }

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
    Eigen::Matrix<T, 3, 3> m_essentialMatrix;
    std::unique_ptr<calibration::KannalaBrandtRadialDistortionModel4<T> > m_leftCameraModel;
    std::unique_ptr<calibration::KannalaBrandtRadialDistortionModel4<T> > m_rightCameraModel;
};

template <typename T> class EpipolarLineVisualizerApp {
public:
    EpipolarLineVisualizerApp(const std::string& leftCameraServerAddress, const std::string& rightCameraServerAddress,
        const std::string& leftTopic, const std::string& rightTopic, const calibration::CameraIntrinsicCalibration& leftIntrinsics,
        const calibration::CameraIntrinsicCalibration& rightIntrinsics, const Eigen::Matrix<T, 3, 3>& rotationMatrix,
        const Eigen::Matrix<T, 3, 1>& translationVector)
        : m_context(2)
        , m_leftCameraSocket(m_context, ZMQ_SUB)
        , m_rightCameraSocket(m_context, ZMQ_SUB)
        , m_visualizer(leftIntrinsics, rightIntrinsics, rotationMatrix, translationVector) {
        const std::string topicleft = leftTopic;
        const std::string topicRight = rightTopic;

        m_leftCameraSocket.setsockopt(ZMQ_RCVHWM, 2);
        m_leftCameraSocket.connect(leftCameraServerAddress);
        m_leftCameraSocket.setsockopt(ZMQ_SUBSCRIBE, topicleft.c_str(), topicleft.size());

        m_rightCameraSocket.setsockopt(ZMQ_RCVHWM, 2);
        m_rightCameraSocket.connect(rightCameraServerAddress);
        m_rightCameraSocket.setsockopt(ZMQ_SUBSCRIBE, topicRight.c_str(), topicRight.size());
    };
    ~EpipolarLineVisualizerApp() = default;

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

            m_visualizer.draw(*leftImage.get(), *rightImage.get());
            m_visualizer.drawEpipolarLine();
            m_visualizer.present();
        }
    }

    zmq::context_t m_context;
    zmq::socket_t m_leftCameraSocket;
    zmq::socket_t m_rightCameraSocket;
    net::ZMQSelectLoop m_selectLoop;
    EpipolarLineVisualization<T> m_visualizer;
};

void SIGINTHandler(int /*signum*/) { s_stopViewer = true; }

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

    EpipolarLineVisualizerApp<float> app(FLAGS_leftCameraAddress, FLAGS_rightCameraAddress, FLAGS_leftTopic, FLAGS_rightTopic,
        leftCameraIntrinsics, rightCameraIntrinsics, rotationMatrix, translationVector);
    app.run();

    return 0;
}
