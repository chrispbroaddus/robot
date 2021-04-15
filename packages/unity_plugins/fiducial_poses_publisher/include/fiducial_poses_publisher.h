#pragma once

#include "packages/net/include/zmq_topic_pub.h"
#include "packages/perception/fiducials/proto/fiducial_poses.pb.h"
#include "packages/unity_plugins/utils/include/vector3.h"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace unity_plugins {

class FiducialPosesPublisher {
public:
    // todo :: Use CameraId IDL
    enum class TargetCoordinate {
        FRONT_FISHEYE_CAM = 0,
        REAR_FISHEYE_CAM = 1,
        LEFT_FISHEYE_CAM = 2,
        RIGHT_FISHEYE_CAM = 3,
        FRONT_LEFT_STEREO_CAM = 4,
        FRONT_RIGHT_STEREO_CAM = 5,
        REAR_LEFT_STEREO_CAM = 6,
        REAR_RIGHT_STEREO_CAM = 7,
        UNITY_WORLD = 8
    };

    // todo (AUTO-530) : use vector3 for rodrigues and translation.
    struct FiducialPose {
        float rodrigues_x;
        float rodrigues_y;
        float rodrigues_z;
        float translation_x;
        float translation_y;
        float translation_z;
    };

    struct FiducialPosesEncoder {
        core::SystemTimestamp systemTimestamp;
        std::vector<FiducialPose> fiducialPoses;
        TargetCoordinate targetCoordinate;
    };

    ///
    /// @brief ZMQ server sending out the Fiducial Poses
    /// \param addr
    /// \param topic
    /// \param port
    /// \param timeoutInMilliseconds
    ///
    FiducialPosesPublisher(const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds);
    FiducialPosesPublisher() = delete;
    FiducialPosesPublisher(const FiducialPosesPublisher& obj) = delete;
    FiducialPosesPublisher& operator=(FiducialPosesPublisher other) = delete;
    FiducialPosesPublisher& operator=(const FiducialPosesPublisher&) = delete;
    FiducialPosesPublisher& operator=(const FiducialPosesPublisher&&) = delete;

    ~FiducialPosesPublisher() = default;

    void clear();

    void add(const FiducialPosesPublisher::TargetCoordinate targetCoordinate, const std::string& familyName, const int fiducialId,
        const FiducialPosesPublisher::FiducialPose& fiducialPose);

    bool send();

private:
    const std::string targetCoordinateInString(const TargetCoordinate targetCoordinate);

    zmq::context_t m_context;
    net::ZMQProtobufPublisher<perception::FiducialPoses> m_publisher;

    std::unique_ptr<perception::FiducialPoses> m_protobufPoses;
};
} // namespace unity_plugins
