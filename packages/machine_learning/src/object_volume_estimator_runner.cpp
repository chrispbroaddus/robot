#include "packages/machine_learning/include/object_volume_estimator_runner.h"

#include "Eigen/Dense"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/json_util.h"

#include "packages/machine_learning/include/rgb_pointcloud_fusion_3d_bbox_estimator.h"
#include "packages/machine_learning/include/rgb_pointcloud_fusion_3d_convex_hull_estimator.h"

#include <fstream>

constexpr char kObjectVolumeEstimatorOptions[] = "config/global/object_volume_estimator_runner_options.pbtxt";

DEFINE_double(depth_percentage, 0.01, "The minimum required percentage to estimate the object volume.");
DEFINE_double(max_distance, 20, "The maximum distance of the stereo-depth data.");
DEFINE_int32(depth_histogram_size, 100, "The size of the depth histogram to find the max value.");

using namespace ml;

ObjectVolumeEstimatorRunner::ObjectVolumeEstimatorRunner(
    const calibration::KannalaBrandtRadialDistortionModel4<double>& kb4Model, const std::string& optionsInPbTxt)
    : m_stop(false)
    , m_kb4Model(kb4Model) {

    // Read configuration
    std::ifstream t(optionsInPbTxt);
    std::stringstream buffer;
    buffer << t.rdbuf();
    CHECK(google::protobuf::TextFormat::MergeFromString(buffer.str(), &m_config));

    /// Initialize the volume estimator
    /// TODO: AUTO-838 Apply real-calibrated parameters into object volume estimator
    m_rotationFisheyeWrtLeftStereo = Eigen::Quaternion<double>(1, 0, 0, 0).toRotationMatrix();
    m_translationFisheyeWrtLeftStereo = Eigen::Matrix<double, 3, 1>(-0.05, 0, 0);
}

ObjectVolumeEstimatorRunner::ObjectVolumeEstimatorRunner(const calibration::KannalaBrandtRadialDistortionModel4<double>& kb4Model)
    : ObjectVolumeEstimatorRunner(kb4Model, kObjectVolumeEstimatorOptions) {}

ObjectVolumeEstimatorRunner::~ObjectVolumeEstimatorRunner() {
    m_stop = true;
    m_thread->join();
}

void ObjectVolumeEstimatorRunner::start() {
    CHECK(!m_thread) << "Start() should not be called more than once.";
    m_thread.reset(new std::thread(&ObjectVolumeEstimatorRunner::estimate, this, std::ref(m_stop)));
}

void ObjectVolumeEstimatorRunner::estimate(std::atomic_bool& stop) {

    // Start stereo depth estimation, streaming data as point cloud.
    LOG(INFO) << "Depth data publisher port: " << m_config.det_port() << " topic: " << m_config.det_topic();

    //  Prepare the context and camera sample subscriber
    zmq::context_t context(1);

    // camera
    std::string detectionSubAddr = "tcp://localhost:" + m_config.det_port();
    net::ZMQProtobufSubscriber<perception::Detection> detectionSubscriber(context, detectionSubAddr, m_config.det_topic(), 1);

    std::string depthSubAddr = "tcp://localhost:" + m_config.depth_port();
    net::ZMQProtobufSubscriber<hal::CameraSample> depthSubscriber(context, depthSubAddr, m_config.depth_topic(), 1);

    std::string volumePubAddr = "tcp://*:" + m_config.obj_volume_port();
    net::ZMQProtobufPublisher<perception::Detection> det3dPublisher(context, volumePubAddr, 1, 1000);

    while (1) {

        /// Data communication
        if (!detectionSubscriber.poll(std::chrono::milliseconds(m_config.sensor_fusion_time_sync_window_threshold_in_milliseconds()))) {
            continue;
        }
        perception::Detection detection;
        detectionSubscriber.recv(detection);

        if (!depthSubscriber.poll(std::chrono::milliseconds(m_config.sensor_fusion_time_sync_window_threshold_in_milliseconds()))) {
            continue;
        }
        hal::CameraSample depthSample;
        depthSubscriber.recv(depthSample);

        CHECK_EQ(depthSample.image().format(), hal::Format::PB_POINTCLOUD);

        auto t1 = std::chrono::high_resolution_clock::now();
        projectPointCloudOnImageSpace(depthSample, detection.box_detection().camera_device_metadata().image_width_pixels(),
            detection.box_detection().camera_device_metadata().image_height_pixels());

        ml::RgbPointcloudFusion3dBboxEstimator bboxEstimator(FLAGS_depth_percentage, FLAGS_depth_histogram_size, FLAGS_max_distance,
            detection.box_detection().camera_device_metadata().image_width_pixels(),
            detection.box_detection().camera_device_metadata().image_height_pixels(), m_kb4Model);

        ml::RgbPointcloudFusion3dConvexHullEstimator convexHullEstimator(FLAGS_depth_percentage, FLAGS_depth_histogram_size,
            FLAGS_max_distance, detection.box_detection().camera_device_metadata().image_width_pixels(),
            detection.box_detection().camera_device_metadata().image_height_pixels(), m_kb4Model);

        /// Meta data
        perception::Detection detection3d;

        perception::CameraAligned3dBoxDetection* box3dDetection = new perception::CameraAligned3dBoxDetection();
        CHECK_NOTNULL(box3dDetection);

        perception::CameraDeviceMetadata* cameraDeviceMetadata = new perception::CameraDeviceMetadata();
        CHECK_NOTNULL(cameraDeviceMetadata);

        cameraDeviceMetadata->ParseFromString(detection.box_detection().camera_device_metadata().SerializeAsString());
        box3dDetection->set_allocated_camera_device_metadata(cameraDeviceMetadata);

        core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
        CHECK_NOTNULL(systemTimestamp);

        systemTimestamp->ParseFromString(detection.box_detection().system_time().SerializeAsString());
        box3dDetection->set_allocated_system_time(systemTimestamp);

        for (auto box : detection.box_detection().bounding_boxes()) {
            ObjectVolumeRepresentation box3dWrapper;
            ObjectVolumeEstimatorInput box2dWrapper(box);

            bool success = bboxEstimator.estimateObjectVolume(box3dWrapper, box2dWrapper, m_depthMapOnImageSpace);
            if (!success) {
                continue;
            }

            auto box3d = box3dDetection->add_bounding_boxes();
            box3d->mutable_category()->CopyFrom(box.category());
            box3d->set_instance_id(box.instance_id());
            box3d->mutable_pose()->CopyFrom(box3dWrapper.box.pose());
            box3d->set_extents_x(box3dWrapper.box.extents_x());
            box3d->set_extents_y(box3dWrapper.box.extents_y());
            box3d->set_extents_z(box3dWrapper.box.extents_z());

            success = convexHullEstimator.estimateObjectVolume(box3dWrapper, box2dWrapper, m_depthMapOnImageSpace);
            if (!success) {
                continue;
            }
            auto convexHull = box3dDetection->add_convex_hulls();
            convexHull->mutable_category()->CopyFrom(box.category());
            convexHull->set_instance_id(box.instance_id());
            convexHull->mutable_pose()->CopyFrom(box3dWrapper.convexHull.pose());
            convexHull->mutable_xs()->CopyFrom(box3dWrapper.convexHull.xs());
            convexHull->mutable_zs()->CopyFrom(box3dWrapper.convexHull.zs());
            convexHull->set_extents_y(box3dWrapper.convexHull.extents_y());
        }
        detection3d.set_allocated_box_3d_detection(box3dDetection);
        det3dPublisher.send(detection3d, "detection3d");

        auto t2 = std::chrono::high_resolution_clock::now();
        // fractional duration: no duration_cast needed
        std::chrono::duration<double, std::milli> fp_ms = t2 - t1;
        LOG(INFO) << __PRETTY_FUNCTION__ << " ... volume estimation took " << fp_ms.count() << " ms";
    }
}

void ObjectVolumeEstimatorRunner::projectPointCloudOnImageSpace(
    const hal::CameraSample& depthSample, const int rgbImageRows, const int rgbImageCols) {

    const size_t depthImageCols = depthSample.image().cols();
    const size_t depthImageRows = depthSample.image().rows();
    const size_t depthImageStride = depthSample.image().stride();

    if (m_depthMapOnImageSpace.size() == 0) {
        m_depthMapOnImageSpace.resize(rgbImageCols * rgbImageRows);
    }
    std::fill(m_depthMapOnImageSpace.begin(), m_depthMapOnImageSpace.end(), -1);

    const float* inputPtr = (const float*)(depthSample.image().data().data());
    CHECK_NOTNULL(inputPtr);

    for (size_t row = 0; row < depthImageRows; row++) {

        // inputStride specifies the number of bytes in a row.
        // Since we are using float pointers and not char pointers we need to adjust the stride accordingly
        const float* inputRowPtr = &inputPtr[row * depthImageStride / 4];

        for (size_t col = 0; col < depthImageCols; col++) {
            Eigen::Map<const Eigen::Matrix<float, 3, 1> > xyz(inputRowPtr + col * 3);
            if (!std::isnan(xyz[2]) && xyz[2] > 0 && xyz[2] < FLAGS_max_distance) {
                Eigen::Matrix<double, 3, 1> xyzp = m_rotationFisheyeWrtLeftStereo * xyz.cast<double>() + m_translationFisheyeWrtLeftStereo;
                auto distortedPoint = m_kb4Model.project(xyzp);
                if ((int)distortedPoint(0) >= 0 && (int)distortedPoint(0) < rgbImageCols && (int)distortedPoint(1) >= 0
                    && (int)distortedPoint(1) < rgbImageRows) {
                    m_depthMapOnImageSpace[(int)distortedPoint(1) * rgbImageCols + (int)distortedPoint(0)]
                        = std::sqrt(xyzp[0] * xyzp[0] + xyzp[1] * xyzp[1] + xyzp[2] * xyzp[2]);
                }
            }
        }
    }
}
