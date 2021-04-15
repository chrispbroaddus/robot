#include "synthetic_fiducial_pose_publisher.h"
#include "glog/logging.h"

#include "packages/core/include/chrono.h"
#include "packages/hal/proto/joystick_sample.pb.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "packages/perception/fiducials/proto/fiducial_poses.pb.h"

using namespace docking;

SyntheticFiducialPosePublisher::SyntheticFiducialPosePublisher(
    const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds, const std::string& topic)
    : m_stop(false)
    , m_thread(&SyntheticFiducialPosePublisher::publish, this, serverAddress, highWaterMark, lingerPeriodInMilliseconds, topic) {}

SyntheticFiducialPosePublisher::~SyntheticFiducialPosePublisher() {
    m_stop = true;
    m_thread.join();
}

void SyntheticFiducialPosePublisher::publish(
    const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds, const std::string& topic) {

    zmq::context_t context(1);
    net::ZMQProtobufPublisher<perception::FiducialPoses> publisher(context, serverAddress, highWaterMark, lingerPeriodInMilliseconds);
    while (!m_stop) {
        perception::FiducialPoses poses;

        const auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();

        hal::Device* targetDevice = new hal::Device();
        targetDevice->set_name("camera");

        calibration::CoordinateFrame* targetFrame = new calibration::CoordinateFrame();
        targetFrame->set_allocated_device(targetDevice);

        hal::Device* sourceDevice = new hal::Device();
        sourceDevice->set_name("test_fiducial");

        calibration::CoordinateFrame* sourceFrame = new calibration::CoordinateFrame();
        sourceFrame->set_allocated_device(sourceDevice);

        auto transformation = poses.add_transformations();

        transformation->set_allocated_sourcecoordinateframe(sourceFrame);
        transformation->set_allocated_targetcoordinateframe(targetFrame);

        float x = 1.f;
        float y = 1.f;
        float z = 1.f;
        float rodrigues_x = M_PI / 2;
        float rodrigues_y = 0;
        float rodrigues_z = 0;

        transformation->set_translationx(x);
        transformation->set_translationy(y);
        transformation->set_translationz(z);
        transformation->set_rodriguesrotationx(rodrigues_x);
        transformation->set_rodriguesrotationy(rodrigues_y);
        transformation->set_rodriguesrotationz(rodrigues_z);

        publisher.send(poses, topic);
    }
}
