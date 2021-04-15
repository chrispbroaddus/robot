#include "packages/machine_learning/include/object_detection_runner.h"
#include "packages/object_tracking/include/sbbc_algorithm.h"
#include "packages/object_tracking/include/sbbc_track.h"
#include "packages/object_tracking/include/track_manager.h"

#include "Eigen/Dense"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/json_util.h"

#include <fstream>

constexpr int kMaxNumFramesPerTrack = 5;
constexpr char kObjectDetectionRunnerOptions[] = "config/global/object_detection_runner_options.front_camera.pbtxt";

using namespace ml;

ObjectDetectionRunner::ObjectDetectionRunner(
    const std::string& modelPath, const calibration::CameraIntrinsicCalibration& calibration, const std::string& options)
    : m_stop(false) {

    // Read configuration
    std::ifstream t(options);
    std::stringstream buffer;
    buffer << t.rdbuf();
    CHECK(google::protobuf::TextFormat::MergeFromString(buffer.str(), &m_config));

    // Un-distort image
    if (m_config.undistort()) {
        m_K.row(0) << calibration.scaledfocallengthx(), 0, calibration.opticalcenterx();
        m_K.row(1) << 0, calibration.scaledfocallengthy(), calibration.opticalcentery();
        m_K.row(2) << 0, 0, 1;

        m_coef << calibration.kannalabrandt().radialdistortioncoefficienti1(), calibration.kannalabrandt().radialdistortioncoefficienti2(),
            calibration.kannalabrandt().radialdistortioncoefficienti3(), calibration.kannalabrandt().radialdistortioncoefficienti4();

        LOG(INFO) << __PRETTY_FUNCTION__ << " ... initializing the undistortion map.";
        m_undistortion = std::make_unique<calibration::Kb4ImageUndistortion<double> >(
            m_K, m_coef, calibration.resolutiony(), calibration.resolutionx(), m_K);
        LOG(INFO) << __PRETTY_FUNCTION__ << "done.";

        m_kb4Model = std::make_unique<calibration::KannalaBrandtRadialDistortionModel4<double> >(m_K, m_coef, 10, 1e-4);
    }

    m_topic = m_config.det_topic();

    // Load object detector model
    m_objectDetector.reset(new ml::ObjectDetector(modelPath, m_config.detection_score_threshold(), ml::CocoLabelToZippyLabelConverter()));
}

ObjectDetectionRunner::ObjectDetectionRunner(const std::string& modelPath, const calibration::CameraIntrinsicCalibration& calibration)
    : ObjectDetectionRunner(modelPath, calibration, kObjectDetectionRunnerOptions) {}

ObjectDetectionRunner::~ObjectDetectionRunner() {
    m_stop = true;
    m_thread->join();
}

void ObjectDetectionRunner::start() {
    CHECK(!m_thread) << "Start() should not be called more than once.";
    m_thread.reset(new std::thread(&ObjectDetectionRunner::detect, this, std::ref(m_stop)));
}

void ObjectDetectionRunner::detect(std::atomic_bool& stop) {

    using SBBCTrack = object_tracking::SimpleBoundingBoxContextTrack;
    std::shared_ptr<object_tracking::SimpleBoundingBoxContextTrackingAlgorithm::MatchParams> sbbcMatchParams;
    sbbcMatchParams.reset(new object_tracking::SimpleBoundingBoxContextTrackingAlgorithm::MatchParams());
    sbbcMatchParams->iouThreshold = 0.7;

    object_tracking::TrackManager<SBBCTrack> trackManager(kMaxNumFramesPerTrack, sbbcMatchParams);

    zmq::context_t context(1);

    // Initialize camera image subscriber
    std::unique_ptr<net::ZMQProtobufSubscriber<hal::CameraSample> > subscriber;
    std::string cameraSubAddr = "tcp://localhost:" + m_config.camera_port();
    subscriber.reset(new net::ZMQProtobufSubscriber<hal::CameraSample>(context, cameraSubAddr, m_config.camera_topic(), 1));

    // Initialize detection result publisher
    std::unique_ptr<net::ZMQProtobufPublisher<perception::Detection> > publisher;
    std::string detPubAddr = "tcp://*:" + m_config.det_pub_port();
    publisher.reset(new net::ZMQProtobufPublisher<perception::Detection>(context, detPubAddr, 1, 100));

    while (!stop) {
        hal::CameraSample cameraSample;

        if (subscriber->poll(std::chrono::milliseconds(10))) {

            subscriber->recv(cameraSample);
            LOG(INFO) << __PRETTY_FUNCTION__ << " ... image size : " << cameraSample.image().cols() << "x" << cameraSample.image().rows();
            LOG(INFO) << __PRETTY_FUNCTION__ << " ... timestamp : " << cameraSample.systemtimestamp().nanos();

            auto t1 = std::chrono::high_resolution_clock::now();

            if (m_config.undistort()) {
                hal::Image undistortedImage;
                m_undistortion->undistortImage(cameraSample.image(), undistortedImage);
                cameraSample.mutable_image()->CopyFrom(undistortedImage);
            }

            auto detection = m_objectDetector->detect(cameraSample);
            object_tracking::uint8_image_t image(cameraSample.image().rows(), cameraSample.image().cols(), cameraSample.image().stride(),
                (unsigned char*)cameraSample.image().data().data());
            trackManager.addFrame(detection, image, cameraSample.hardwaretimestamp());

            if (m_config.undistort()) {
                distortBoundingBoxes(detection);
            }

            auto t2 = std::chrono::high_resolution_clock::now();
            // fractional duration: no duration_cast needed
            std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

            LOG(INFO) << __PRETTY_FUNCTION__ << " ... detection took " << fp_ms.count() << " ms";

            LOG(INFO) << __PRETTY_FUNCTION__ << " ... # of detections : " << detection.box_detection().bounding_boxes_size();

            for (int i = 0; i < std::min(detection.box_detection().bounding_boxes_size(), 3); i++) {
                switch (detection.box_detection().bounding_boxes(i).category().type()) {
                case perception::Category::PERSON:
                    LOG(INFO) << __PRETTY_FUNCTION__ << " ... ... detected a person.";
                    break;
                case perception::Category::CAR:
                    LOG(INFO) << __PRETTY_FUNCTION__ << " ... ... detected a car.";
                    break;
                case perception::Category::BICYCLE:
                    LOG(INFO) << __PRETTY_FUNCTION__ << " ... ... detected a bicycle.";
                    break;
                case perception::Category::BUS:
                    LOG(INFO) << __PRETTY_FUNCTION__ << " ... ... detected a bus.";
                    break;
                case perception::Category::TRUCK:
                    LOG(INFO) << __PRETTY_FUNCTION__ << " ... ... detected a truck.";
                    break;
                default:
                    LOG(INFO) << __PRETTY_FUNCTION__ << " ... ... detected something else.";
                    break;
                }
            }

            if (detection.box_detection().bounding_boxes_size() > 0) {
                detection.mutable_box_detection()->mutable_camera_device_metadata()->mutable_device_metadata()->mutable_device()->set_name(
                    m_config.camera_topic());
                publisher->send(detection, m_topic);
            }
        }
    }
    LOG(WARNING) << __PRETTY_FUNCTION__ << "The detection is done.";
}

void ObjectDetectionRunner::distortBoundingBoxes(perception::Detection& detection) {

    perception::CameraAlignedBoxDetection* boxDetection = new perception::CameraAlignedBoxDetection();
    CHECK_NOTNULL(boxDetection);

    boxDetection->mutable_camera_device_metadata()->CopyFrom(detection.box_detection().camera_device_metadata());
    boxDetection->mutable_system_time()->CopyFrom(detection.box_detection().system_time());

    for (int i = 0; i < detection.box_detection().bounding_boxes_size(); i++) {

        auto distortedBox = boxDetection->add_bounding_boxes();
        auto& box = detection.box_detection().bounding_boxes(i);

        auto x = box.top_left_x();
        auto y = box.top_left_y();
        auto w = box.extents_x();
        auto h = box.extents_y();

        distortedBox->mutable_category()->CopyFrom(box.category());
        distortedBox->mutable_motion()->CopyFrom(box.motion());
        distortedBox->set_instance_id(box.instance_id());

        // top-left
        Eigen::Matrix<double, 3, 1> tlUndistortedPoint;
        tlUndistortedPoint << (x - m_K(0, 2)) / m_K(0, 0), (y - m_K(1, 2)) / m_K(1, 1), 1;
        auto tlDistortedPoint = m_kb4Model->project(tlUndistortedPoint);
        distortedBox->set_top_left_x(tlDistortedPoint(0));
        distortedBox->set_top_left_y(tlDistortedPoint(1));

        // bottom-right
        Eigen::Matrix<double, 3, 1> brUndistortedPoint;
        brUndistortedPoint << (x + w - m_K(0, 2)) / m_K(0, 0), (y + h - m_K(1, 2)) / m_K(1, 1), 1;
        auto brDistortedPoint = m_kb4Model->project(brUndistortedPoint);
        distortedBox->set_extents_x(brDistortedPoint(0) - distortedBox->top_left_x());
        distortedBox->set_extents_y(brDistortedPoint(1) - distortedBox->top_left_y());
    }

    detection.clear_box_detection();
    detection.set_allocated_box_detection(boxDetection);
}
