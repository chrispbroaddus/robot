#include "SDL.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/util/json_util.h"
#include "packages/calibration/include/kannala_brandt_distortion_model.h"
#include "packages/calibration/proto/camera_intrinsic_calibration.pb.h"
#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/core/include/aligned_memory_allocator.h"
#include "packages/core/include/dynamic_image.h"
#include "packages/core/include/dynamic_storage.h"
#include "packages/core/include/pixel_layout.h"
#include "packages/hal/proto/camera_id.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_topic_sub.h"
#include "packages/perception/proto/detection.pb.h"
#include <fstream>

DEFINE_string(ip, "", "The IP address where image and detection data comes from.");
DEFINE_string(image_port, "", "The port number where camera sample is published out.");
DEFINE_string(det_port, "", "The port number where detection result is published out.");
DEFINE_string(det3d_port, "", "[Optional] The port number where detection result is published out.");
DEFINE_string(image_topic, "camera", "The topic of image samples used for detection.");
DEFINE_string(det_topic, "detection", "The topic of detection samples.");
DEFINE_string(systemCalibrationFile, "", "system calibration file");

void checkGFLAGSInput() {
    if (FLAGS_ip.empty()) {
        throw std::runtime_error("The flag -model is required.");
    }

    if (FLAGS_image_port.empty()) {
        throw std::runtime_error("The flag -image_port is required.");
    }

    if (FLAGS_det_port.empty()) {
        throw std::runtime_error("The flag -det_port is required.");
    }

    if (FLAGS_image_topic.empty()) {
        LOG(WARNING) << __PRETTY_FUNCTION__ << " ... using the default image_topic : " << FLAGS_image_topic;
    }

    if (FLAGS_det_topic.empty()) {
        LOG(WARNING) << __PRETTY_FUNCTION__ << " ... using the default det_topic : " << FLAGS_det_topic;
    }

    if (FLAGS_systemCalibrationFile.empty()) {
        throw std::runtime_error("The flag -systemCalibrationFile is required.");
    }
}

namespace details {
typedef core::DynamicStorage<core::rgb8_scalar_t, core::AlignedMemoryAllocator> rgb8_aligned_storage_t;
typedef core::DynamicImage<core::ImageType::rgb8, details::rgb8_aligned_storage_t> rgb8_image_t;
}

///
/// \brief Draw a SDL line with a specified line width
///
void SDL_RenderDrawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int lineWidth);

///
/// \brief Draw a 2D Bounding Box over the detected object
///
void draw2dBoundingBox(SDL_Renderer* renderer, const perception::ObjectBoundingBox& box, const int imageRows, const int imageCols);

///
/// \brief Draw a 3D Bounding Box over the detected object
///
void draw3dBoundingBox(SDL_Renderer* renderer, calibration::KannalaBrandtRadialDistortionModel4<double> lensModel,
    const perception::Object3dBoundingBox& box, const int imageRows, const int imageCols);

///
/// \brief Draw a 3D Convex Hull over the detected object
///
void draw3dConvexHull(SDL_Renderer* renderer, calibration::KannalaBrandtRadialDistortionModel4<double> lensModel,
    const perception::Object3dConvexHull& convexHull, const int imageRows, const int imageCols);

int main(int argc, char** argv) {

    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    gflags::ParseCommandLineFlags(&argc, &argv, false);
    FLAGS_logtostderr = true;

    checkGFLAGSInput();

    calibration::CameraIntrinsicCalibration fisheyeIntrinsic;
    bool foundFisheyeIntrinsic = false;
    calibration::SystemCalibration systemCalibration;

    std::ifstream file(FLAGS_systemCalibrationFile);
    std::stringstream buffer;
    buffer << file.rdbuf();
    if (buffer.str().size() == 0) {
        LOG(ERROR) << "Empty system calibration file";
        throw std::runtime_error("Empty system calibration file");
    }
    google::protobuf::util::JsonStringToMessage(buffer.str(), &systemCalibration);

    for (int i = 0; i < systemCalibration.cameraintrinsiccalibration().size(); i++) {
        hal::CameraId detectedCameraId;
        bool parseSuccess
            = hal::CameraId_Parse(systemCalibration.cameraintrinsiccalibration(i).cameraundercalibration().name(), &detectedCameraId);
        if (parseSuccess && detectedCameraId == hal::CameraId::FrontFisheye) {
            foundFisheyeIntrinsic = true;
            fisheyeIntrinsic = systemCalibration.cameraintrinsiccalibration(i);
        }
        CHECK(foundFisheyeIntrinsic) << "The system calibration does not have intrinsic parameters for the fisheye lens.";
    }

    Eigen::Matrix<double, 3, 3> K;
    K.row(0) << fisheyeIntrinsic.scaledfocallengthx(), 0, fisheyeIntrinsic.opticalcenterx();
    K.row(1) << 0, fisheyeIntrinsic.scaledfocallengthy(), fisheyeIntrinsic.opticalcentery();
    K.row(2) << 0, 0, 1;
    Eigen::Matrix<double, 4, 1> coef;
    coef << fisheyeIntrinsic.kannalabrandt().radialdistortioncoefficientk(0),
        fisheyeIntrinsic.kannalabrandt().radialdistortioncoefficientk(1), fisheyeIntrinsic.kannalabrandt().radialdistortioncoefficientk(2),
        fisheyeIntrinsic.kannalabrandt().radialdistortioncoefficientk(3);

    calibration::KannalaBrandtRadialDistortionModel4<double> lensModel(K, coef, 10, 1e-10);

    zmq::context_t context(1);
    std::string imageSamplePubAddr = "tcp://" + FLAGS_ip + ":" + FLAGS_image_port;
    net::ZMQProtobufSubscriber<hal::CameraSample> imageSubscriber(context, imageSamplePubAddr, FLAGS_image_topic, 1);

    std::string detPubAddr = "tcp://" + FLAGS_ip + ":" + FLAGS_det_port;
    net::ZMQProtobufSubscriber<perception::Detection> detectionSubscriber(context, detPubAddr, FLAGS_det_topic, 1);

    std::unique_ptr<net::ZMQProtobufSubscriber<perception::Detection> > detection3dSubscriber;
    if (!FLAGS_det3d_port.empty()) {
        std::string det3dPubAddr = "tcp://" + FLAGS_ip + ":" + FLAGS_det3d_port;
        detection3dSubscriber.reset(new net::ZMQProtobufSubscriber<perception::Detection>(context, det3dPubAddr, "detection3d", 1));
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;

    int imageRows = 0;
    int imageCols = 0;

    SDL_Event event;

    enum State { NOT_RUNNING = 0, SHOW_2D_ONLY = 1, SHOW_3D_BBOX_ONLY = 2, SHOW_3D_CONVEX_HULL_ONLY = 3, SHOW_ALL = 4 };

    State state = SHOW_2D_ONLY;
    auto tLastInput = std::chrono::high_resolution_clock::now();

    while (state != NOT_RUNNING) {

        if (SDL_PollEvent(&event)) {
            // fractional duration: no duration_cast needed
            auto tNow = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> fp_ms = tNow - tLastInput;

            if (event.type == SDL_KEYDOWN && fp_ms > std::chrono::duration<double, std::milli>(10)) {
                tLastInput = tNow;
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    state = NOT_RUNNING;
                    break;
                }
                switch (state) {
                case SHOW_2D_ONLY:
                    state = SHOW_3D_BBOX_ONLY;
                    break;

                case SHOW_3D_BBOX_ONLY:
                    state = SHOW_3D_CONVEX_HULL_ONLY;
                    break;

                case SHOW_3D_CONVEX_HULL_ONLY:
                    state = SHOW_ALL;
                    break;

                case SHOW_ALL:
                    state = SHOW_2D_ONLY;
                    break;

                default:
                    break;
                }
            }
        }

        if (!imageSubscriber.poll(std::chrono::milliseconds(1)) || !detectionSubscriber.poll(std::chrono::milliseconds(1))) {
            continue;
        }
        hal::CameraSample cameraSample;
        imageSubscriber.recv(cameraSample);

        if (!detectionSubscriber.poll(std::chrono::milliseconds(1))) {
            continue;
        }
        perception::Detection detection;
        detectionSubscriber.recv(detection);

        if (detection3dSubscriber && !detection3dSubscriber->poll(std::chrono::milliseconds(1))) {
            continue;
        }
        perception::Detection detection3d;
        if (detection3dSubscriber) {
            detection3dSubscriber->recv(detection3d);
        }

        if (!window) {
            imageRows = cameraSample.image().rows();
            imageCols = cameraSample.image().cols();

            window = SDL_CreateWindow(
                "object detection", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, imageCols / 3, imageRows / 3, SDL_WINDOW_RESIZABLE);
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, imageCols, imageRows);
            SDL_SetRenderTarget(renderer, texture);
        }

        core::DynamicImage<core::ImageType::rgb8, details::rgb8_aligned_storage_t> image(
            cameraSample.image().rows(), cameraSample.image().cols());
        auto view = image.view();

        for (size_t row = 0; row < cameraSample.image().rows(); row++) {
            core::rgb8_pixel* textureRowPixels = (core::rgb8_pixel*)&view.data[row * view.stride];
            for (size_t col = 0; col < cameraSample.image().cols(); col++) {
                // Left image
                textureRowPixels[col].r = cameraSample.image().data()[row * cameraSample.image().stride() + col * 3];
                textureRowPixels[col].g = cameraSample.image().data()[row * cameraSample.image().stride() + col * 3 + 1];
                textureRowPixels[col].b = cameraSample.image().data()[row * cameraSample.image().stride() + col * 3 + 2];
            }
        }

        CHECK(SDL_UpdateTexture(texture, NULL, view.data, view.stride) == 0);
        CHECK(SDL_RenderCopy(renderer, texture, NULL, NULL) == 0);

        int sdlWindowWidth, sdlWindowHeight;
        SDL_GetRendererOutputSize(renderer, &sdlWindowWidth, &sdlWindowHeight);

        if (state == SHOW_2D_ONLY || state == SHOW_ALL) {
            for (int i = 0; i < detection.box_detection().bounding_boxes_size(); i++) {

                const auto& box = detection.box_detection().bounding_boxes(i);
                draw2dBoundingBox(renderer, box, imageRows, imageCols);
            }
        }

        if (state == SHOW_3D_BBOX_ONLY || state == SHOW_ALL) {
            for (int i = 0; i < detection3d.box_3d_detection().bounding_boxes_size(); i++) {
                const auto& box = detection3d.box_3d_detection().bounding_boxes(i);
                draw3dBoundingBox(renderer, lensModel, box, imageRows, imageCols);
            }
        }

        if (state == SHOW_3D_CONVEX_HULL_ONLY || state == SHOW_ALL) {
            for (int i = 0; i < detection3d.box_3d_detection().convex_hulls_size(); i++) {
                const auto& convexHull = detection3d.box_3d_detection().convex_hulls(i);
                draw3dConvexHull(renderer, lensModel, convexHull, imageRows, imageCols);
            }
        }

        SDL_RenderPresent(renderer);
        SDL_PollEvent(nullptr);
    }
}

void SDL_RenderDrawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int lineWidth) {
    int lineWidthHalf = lineWidth / 2;
    for (int i = -lineWidthHalf; i <= lineWidthHalf; i++) {
        for (int j = -lineWidthHalf; j <= lineWidthHalf; j++) {
            SDL_RenderDrawLine(renderer, x1 + i, y1 + j, x2 + i, y2 + j);
        }
    }
}

void draw2dBoundingBox(SDL_Renderer* renderer, const perception::ObjectBoundingBox& box, const int imageRows, const int imageCols) {

    switch (box.category().type()) {
    case perception::Category::PERSON:
        if (box.instance_id() % 5 == 0) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 128, 255);
        } else if (box.instance_id() % 5 == 1) {
            SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
        } else if (box.instance_id() % 5 == 2) {
            SDL_SetRenderDrawColor(renderer, 0, 128, 128, 255);
        } else if (box.instance_id() % 5 == 3) {
            SDL_SetRenderDrawColor(renderer, 128, 0, 0, 255);
        } else if (box.instance_id() % 5 == 4) {
            SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255);
        }
        break;

    case perception::Category::CAR:
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        break;

    case perception::Category::TRUCK:
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        break;

    case perception::Category::BUS:
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        break;

    default:
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        break;
    }

    int sdlWindowWidth, sdlWindowHeight;
    SDL_GetRendererOutputSize(renderer, &sdlWindowWidth, &sdlWindowHeight);

    // Draw a bounding box with lineWidth=3
    SDL_Rect rect;
    rect.x = box.top_left_x() * (double)sdlWindowWidth / imageCols;
    rect.y = box.top_left_y() * (double)sdlWindowHeight / imageRows;
    rect.w = box.extents_x() * (double)sdlWindowWidth / imageCols;
    rect.h = box.extents_y() * (double)sdlWindowHeight / imageRows;

    auto rectLarge = rect;
    rectLarge.x -= 1;
    rectLarge.y -= 1;
    rectLarge.w += 2;
    rectLarge.h += 2;

    auto rectSmall = rect;
    rectSmall.x += 1;
    rectSmall.y += 1;
    rectSmall.w -= 2;
    rectSmall.h -= 2;

    SDL_RenderDrawRect(renderer, &rect);
    SDL_RenderDrawRect(renderer, &rectLarge);
    SDL_RenderDrawRect(renderer, &rectSmall);
}

void draw3dBoundingBox(SDL_Renderer* renderer, calibration::KannalaBrandtRadialDistortionModel4<double> lensModel,
    const perception::Object3dBoundingBox& box, const int imageRows, const int imageCols) {

    Eigen::Matrix<double, 3, 1> x1, x2, x3, x4, x5, x6, x7, x8;
    x1 << box.pose().translationx() - box.extents_x() / 2, box.pose().translationy() - box.extents_y() / 2,
        box.pose().translationz() - box.extents_z() / 2;
    x2 << box.pose().translationx() - box.extents_x() / 2, box.pose().translationy() - box.extents_y() / 2,
        box.pose().translationz() + box.extents_z() / 2;
    x3 << box.pose().translationx() + box.extents_x() / 2, box.pose().translationy() - box.extents_y() / 2,
        box.pose().translationz() + box.extents_z() / 2;
    x4 << box.pose().translationx() + box.extents_x() / 2, box.pose().translationy() - box.extents_y() / 2,
        box.pose().translationz() - box.extents_z() / 2;
    x5 << box.pose().translationx() - box.extents_x() / 2, box.pose().translationy() + box.extents_y() / 2,
        box.pose().translationz() - box.extents_z() / 2;
    x6 << box.pose().translationx() - box.extents_x() / 2, box.pose().translationy() + box.extents_y() / 2,
        box.pose().translationz() + box.extents_z() / 2;
    x7 << box.pose().translationx() + box.extents_x() / 2, box.pose().translationy() + box.extents_y() / 2,
        box.pose().translationz() + box.extents_z() / 2;
    x8 << box.pose().translationx() + box.extents_x() / 2, box.pose().translationy() + box.extents_y() / 2,
        box.pose().translationz() - box.extents_z() / 2;

    auto x1p = lensModel.project(x1);
    auto x2p = lensModel.project(x2);
    auto x3p = lensModel.project(x3);
    auto x4p = lensModel.project(x4);
    auto x5p = lensModel.project(x5);
    auto x6p = lensModel.project(x6);
    auto x7p = lensModel.project(x7);
    auto x8p = lensModel.project(x8);

    switch (box.category().type()) {
    case perception::Category::PERSON:
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        break;

    case perception::Category::CAR:
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        break;

    case perception::Category::TRUCK:
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        break;

    case perception::Category::BUS:
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        break;

    default:
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        break;
    }

    int sdlWindowWidth, sdlWindowHeight;
    SDL_GetRendererOutputSize(renderer, &sdlWindowWidth, &sdlWindowHeight);
    double rx = (double)sdlWindowWidth / imageCols;
    double ry = (double)sdlWindowHeight / imageRows;

    int lineWidth = 5;
    SDL_RenderDrawLine(renderer, (int)(x1p(0) * rx), (int)(x1p(1) * ry), (int)(x2p(0) * rx), (int)(x2p(1) * ry), lineWidth);
    SDL_RenderDrawLine(renderer, (int)(x2p(0) * rx), (int)(x2p(1) * ry), (int)(x3p(0) * rx), (int)(x3p(1) * ry), lineWidth);
    SDL_RenderDrawLine(renderer, (int)(x3p(0) * rx), (int)(x3p(1) * ry), (int)(x4p(0) * rx), (int)(x4p(1) * ry), lineWidth);
    SDL_RenderDrawLine(renderer, (int)(x4p(0) * rx), (int)(x4p(1) * ry), (int)(x1p(0) * rx), (int)(x1p(1) * ry), lineWidth);

    SDL_RenderDrawLine(renderer, (int)(x5p(0) * rx), (int)(x5p(1) * ry), (int)(x6p(0) * rx), (int)(x6p(1) * ry), lineWidth);
    SDL_RenderDrawLine(renderer, (int)(x6p(0) * rx), (int)(x6p(1) * ry), (int)(x7p(0) * rx), (int)(x7p(1) * ry), lineWidth);
    SDL_RenderDrawLine(renderer, (int)(x7p(0) * rx), (int)(x7p(1) * ry), (int)(x8p(0) * rx), (int)(x8p(1) * ry), lineWidth);
    SDL_RenderDrawLine(renderer, (int)(x8p(0) * rx), (int)(x8p(1) * ry), (int)(x5p(0) * rx), (int)(x5p(1) * ry), lineWidth);

    SDL_RenderDrawLine(renderer, (int)(x1p(0) * rx), (int)(x1p(1) * ry), (int)(x5p(0) * rx), (int)(x5p(1) * ry), lineWidth);
    SDL_RenderDrawLine(renderer, (int)(x2p(0) * rx), (int)(x2p(1) * ry), (int)(x6p(0) * rx), (int)(x6p(1) * ry), lineWidth);
    SDL_RenderDrawLine(renderer, (int)(x3p(0) * rx), (int)(x3p(1) * ry), (int)(x7p(0) * rx), (int)(x7p(1) * ry), lineWidth);
    SDL_RenderDrawLine(renderer, (int)(x4p(0) * rx), (int)(x4p(1) * ry), (int)(x8p(0) * rx), (int)(x8p(1) * ry), lineWidth);

    if (box.category().type() == perception::Category::PERSON) {
        if (box.instance_id() % 5 == 0) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 128, 255);
        } else if (box.instance_id() % 5 == 1) {
            SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
        } else if (box.instance_id() % 5 == 2) {
            SDL_SetRenderDrawColor(renderer, 0, 128, 128, 255);
        } else if (box.instance_id() % 5 == 3) {
            SDL_SetRenderDrawColor(renderer, 128, 0, 0, 255);
        } else if (box.instance_id() % 5 == 4) {
            SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255);
        }
        lineWidth = 3;
        SDL_RenderDrawLine(renderer, (int)(x1p(0) * rx), (int)(x1p(1) * ry), (int)(x2p(0) * rx), (int)(x2p(1) * ry), lineWidth);
        SDL_RenderDrawLine(renderer, (int)(x2p(0) * rx), (int)(x2p(1) * ry), (int)(x3p(0) * rx), (int)(x3p(1) * ry), lineWidth);
        SDL_RenderDrawLine(renderer, (int)(x3p(0) * rx), (int)(x3p(1) * ry), (int)(x4p(0) * rx), (int)(x4p(1) * ry), lineWidth);
        SDL_RenderDrawLine(renderer, (int)(x4p(0) * rx), (int)(x4p(1) * ry), (int)(x1p(0) * rx), (int)(x1p(1) * ry), lineWidth);

        SDL_RenderDrawLine(renderer, (int)(x5p(0) * rx), (int)(x5p(1) * ry), (int)(x6p(0) * rx), (int)(x6p(1) * ry), lineWidth);
        SDL_RenderDrawLine(renderer, (int)(x6p(0) * rx), (int)(x6p(1) * ry), (int)(x7p(0) * rx), (int)(x7p(1) * ry), lineWidth);
        SDL_RenderDrawLine(renderer, (int)(x7p(0) * rx), (int)(x7p(1) * ry), (int)(x8p(0) * rx), (int)(x8p(1) * ry), lineWidth);
        SDL_RenderDrawLine(renderer, (int)(x8p(0) * rx), (int)(x8p(1) * ry), (int)(x5p(0) * rx), (int)(x5p(1) * ry), lineWidth);

        SDL_RenderDrawLine(renderer, (int)(x1p(0) * rx), (int)(x1p(1) * ry), (int)(x5p(0) * rx), (int)(x5p(1) * ry), lineWidth);
        SDL_RenderDrawLine(renderer, (int)(x2p(0) * rx), (int)(x2p(1) * ry), (int)(x6p(0) * rx), (int)(x6p(1) * ry), lineWidth);
        SDL_RenderDrawLine(renderer, (int)(x3p(0) * rx), (int)(x3p(1) * ry), (int)(x7p(0) * rx), (int)(x7p(1) * ry), lineWidth);
        SDL_RenderDrawLine(renderer, (int)(x4p(0) * rx), (int)(x4p(1) * ry), (int)(x8p(0) * rx), (int)(x8p(1) * ry), lineWidth);
    }
}

void draw3dConvexHull(SDL_Renderer* renderer, calibration::KannalaBrandtRadialDistortionModel4<double> lensModel,
    const perception::Object3dConvexHull& convexHull, const int imageRows, const int imageCols) {

    std::vector<Eigen::Matrix<double, 2, 1> > upperPoints;
    std::vector<Eigen::Matrix<double, 2, 1> > lowerPoints;

    for (int i = 0; i < convexHull.xs_size(); i++) {
        const auto& x = convexHull.xs(i);
        const auto& z = convexHull.zs(i);
        Eigen::Matrix<double, 3, 1> pu(x, convexHull.pose().translationy() - convexHull.extents_y() / 2, z);
        Eigen::Matrix<double, 3, 1> pl(x, convexHull.pose().translationy() + convexHull.extents_y() / 2, z);
        upperPoints.push_back(lensModel.project(pu));
        lowerPoints.push_back(lensModel.project(pl));
    }

    switch (convexHull.category().type()) {
    case perception::Category::PERSON:
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        break;

    case perception::Category::CAR:
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        break;

    case perception::Category::TRUCK:
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        break;

    case perception::Category::BUS:
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        break;

    default:
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        break;
    }

    int sdlWindowWidth, sdlWindowHeight;
    SDL_GetRendererOutputSize(renderer, &sdlWindowWidth, &sdlWindowHeight);
    double rx = (double)sdlWindowWidth / imageCols;
    double ry = (double)sdlWindowHeight / imageRows;

    int lineWidth = 3;
    for (int i = 0; i < convexHull.xs_size(); i++) {
        int c = i;
        int n = i + 1;
        if (n == convexHull.xs_size()) {
            n = 0;
        }
        SDL_RenderDrawLine(renderer, (int)(upperPoints[c](0) * rx), (int)(upperPoints[c](1) * ry), (int)(upperPoints[n](0) * rx),
            (int)(upperPoints[n](1) * ry), lineWidth);
        SDL_RenderDrawLine(renderer, (int)(lowerPoints[c](0) * rx), (int)(lowerPoints[c](1) * ry), (int)(lowerPoints[n](0) * rx),
            (int)(lowerPoints[n](1) * ry), lineWidth);
        SDL_RenderDrawLine(renderer, (int)(upperPoints[c](0) * rx), (int)(upperPoints[c](1) * ry), (int)(lowerPoints[c](0) * rx),
            (int)(lowerPoints[c](1) * ry), lineWidth);
    }

    if (convexHull.category().type() == perception::Category::PERSON) {
        if (convexHull.instance_id() % 5 == 0) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 128, 255);
        } else if (convexHull.instance_id() % 5 == 1) {
            SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);
        } else if (convexHull.instance_id() % 5 == 2) {
            SDL_SetRenderDrawColor(renderer, 0, 128, 128, 255);
        } else if (convexHull.instance_id() % 5 == 3) {
            SDL_SetRenderDrawColor(renderer, 128, 0, 0, 255);
        } else if (convexHull.instance_id() % 5 == 4) {
            SDL_SetRenderDrawColor(renderer, 128, 0, 128, 255);
        }
        int lineWidth = 1;
        for (int i = 0; i < convexHull.xs_size(); i++) {
            int c = i;
            int n = i + 1;
            if (n == convexHull.xs_size()) {
                n = 0;
            }
            SDL_RenderDrawLine(renderer, (int)(upperPoints[c](0) * rx), (int)(upperPoints[c](1) * ry), (int)(upperPoints[n](0) * rx),
                (int)(upperPoints[n](1) * ry), lineWidth);
            SDL_RenderDrawLine(renderer, (int)(lowerPoints[c](0) * rx), (int)(lowerPoints[c](1) * ry), (int)(lowerPoints[n](0) * rx),
                (int)(lowerPoints[n](1) * ry), lineWidth);
            SDL_RenderDrawLine(renderer, (int)(upperPoints[c](0) * rx), (int)(upperPoints[c](1) * ry), (int)(lowerPoints[c](0) * rx),
                (int)(lowerPoints[c](1) * ry), lineWidth);
        }
    }
}