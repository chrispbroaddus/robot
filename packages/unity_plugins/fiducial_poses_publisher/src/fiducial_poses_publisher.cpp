#include "packages/unity_plugins/fiducial_poses_publisher/include/fiducial_poses_publisher.h"
#include "Eigen/Eigen"
#include "glog/logging.h"
#include "packages/core/include/chrono.h"
#include "packages/unity_plugins/utils/include/coordinate_conversion.h"
#include "packages/unity_plugins/utils/include/zippy_interface.h"

using namespace unity_plugins;

static std::unique_ptr<FiducialPosesPublisher> s_server;

extern "C" {
ZIPPY_INTERFACE_EXPORT int FiducialPosesPublisher_initializeApriltagPosePublisher(
    const char* addr, const int highWaterMark, const int lingerPeriodInMilliseconds) {
    std::string serverAddress(addr);
    s_server.reset(new FiducialPosesPublisher(serverAddress, highWaterMark, lingerPeriodInMilliseconds));
    return 1;
}

ZIPPY_INTERFACE_EXPORT void FiducialPosesPublisher_add(FiducialPosesPublisher::TargetCoordinate targetCoordinate, const char* familyName,
    const int fiducialId, FiducialPosesPublisher::FiducialPose fiducialPose) {
    LOG(INFO) << __PRETTY_FUNCTION__ << ", poseName : " << std::string(familyName);
    s_server->add(targetCoordinate, std::string(familyName), fiducialId, fiducialPose);
}

ZIPPY_INTERFACE_EXPORT int FiducialPosesPublisher_send() {
    if (s_server->send()) {
        LOG(INFO) << "Sending fiducial pose ... success.";
        return 1;
    } else {
        LOG(INFO) << "Sending fiducial pose ... failed.";
        return 0;
    }
}

ZIPPY_INTERFACE_EXPORT void FiducialPosesPublisher_clear() { s_server->clear(); }

ZIPPY_INTERFACE_EXPORT void FiducialPosesPublisher_stop() {
    if (s_server) {
        s_server.reset();
    }
}
}

FiducialPosesPublisher::FiducialPosesPublisher(
    const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds)
    : m_context(1)
    , m_publisher(m_context, serverAddress, highWaterMark, lingerPeriodInMilliseconds)
    , m_protobufPoses(new perception::FiducialPoses()) {}

bool FiducialPosesPublisher::send() {
    const auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();
    m_protobufPoses->mutable_system_timestamp()->set_nanos(gpsTimestamp.count());
    LOG(INFO) << "Sending out fiducial poses...";
    return m_publisher.send(*m_protobufPoses, "fiducial_poses");
}

void FiducialPosesPublisher::clear() { m_protobufPoses->clear_transformations(); }

void FiducialPosesPublisher::add(const FiducialPosesPublisher::TargetCoordinate targetCoordinate, const std::string& familyName,
    const int fiducialId, const FiducialPosesPublisher::FiducialPose& pose) {

    CHECK_NOTNULL(m_protobufPoses.get());

    auto t = m_protobufPoses->add_transformations();

    CHECK_NOTNULL(t);

    float rx, ry, rz, tx, ty, tz;
    convertUnityAprilTagToDockingAprilTagCoordinate(rx, ry, rz, tx, ty, tz, pose.rodrigues_x, pose.rodrigues_y, pose.rodrigues_z,
        pose.translation_x, pose.translation_y, pose.translation_z);

    if (tx < 0) {
        LOG(INFO) << "WARNING : The fiducial is behind the camera, which could be a bug.";
    }

    t->set_translationx(tx);
    t->set_translationy(ty);
    t->set_translationz(tz);
    t->set_rodriguesrotationx(rx);
    t->set_rodriguesrotationy(ry);
    t->set_rodriguesrotationz(rz);

    hal::Device* targetWorldDevice = new hal::Device();
    CHECK_NOTNULL(targetWorldDevice);
    targetWorldDevice->set_name(targetCoordinateInString(targetCoordinate));
    targetWorldDevice->set_serialnumber(0x0000000000000000); // todo : https://zippyai.atlassian.net/browse/AUTO-508
    calibration::CoordinateFrame* targetWorldCoord = new calibration::CoordinateFrame();
    targetWorldCoord->set_allocated_device(targetWorldDevice);
    t->set_allocated_targetcoordinateframe(targetWorldCoord);

    hal::Device* fiducialDevice = new hal::Device();
    CHECK_NOTNULL(fiducialDevice);
    fiducialDevice->set_name(familyName);
    fiducialDevice->set_serialnumber(fiducialId);
    calibration::CoordinateFrame* fiducialDeviceCoord = new calibration::CoordinateFrame();
    fiducialDeviceCoord->set_allocated_device(fiducialDevice);
    t->set_allocated_sourcecoordinateframe(fiducialDeviceCoord);
}

const std::string FiducialPosesPublisher::targetCoordinateInString(const TargetCoordinate targetCoordinate) {
    switch (targetCoordinate) {
    case TargetCoordinate::FRONT_FISHEYE_CAM:
        return "FRONT_FISHEYE_CAM";
    case TargetCoordinate::REAR_FISHEYE_CAM:
        return "REAR_FISHEYE_CAM";
    case TargetCoordinate::LEFT_FISHEYE_CAM:
        return "LEFT_FISHEYE_CAM";
    case TargetCoordinate::RIGHT_FISHEYE_CAM:
        return "RIGHT_FISHEYE_CAM";
    case TargetCoordinate::FRONT_LEFT_STEREO_CAM:
        return "FRONT_LEFT_STEREO_CAM";
    case TargetCoordinate::FRONT_RIGHT_STEREO_CAM:
        return "FRONT_RIGHT_STEREO_CAM";
    case TargetCoordinate::REAR_LEFT_STEREO_CAM:
        return "REAR_LEFT_STEREO_CAM";
    case TargetCoordinate::REAR_RIGHT_STEREO_CAM:
        return "REAR_RIGHT_STEREO_CAM";
    case TargetCoordinate::UNITY_WORLD:
        return "UNITY_WORLD";
    default:
        CHECK(false);
        return "UNSUPPORTED_COORDINATE";
    }
}
