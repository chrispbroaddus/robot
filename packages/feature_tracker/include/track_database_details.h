#pragma once

#include "Eigen/Dense"
#include "packages/feature_detectors/proto/feature_point.pb.h"
#include <unordered_map>
#include <unordered_set>

namespace feature_tracker {

/// Represents a 3D point
template <typename POINT_T> class Point3d {
public:
    Point3d()
        : m_set(false) {}
    Point3d(const Eigen::Matrix<POINT_T, 3, 1>& x)
        : m_set(true)
        , m_point(x) {}
    bool m_set;
    Eigen::Matrix<POINT_T, 3, 1> m_point;
};

/// Represents a pose
template <typename SCALAR> class Pose {
public:
    Pose()
        : m_set(false) {}
    Pose(const Eigen::Quaternion<SCALAR>& q, const Eigen::Matrix<SCALAR, 3, 1>& p)
        : m_set(true)
        , m_quaternion(q)
        , m_position(p) {}

    bool m_set;
    Eigen::Quaternion<SCALAR> m_quaternion;
    Eigen::Matrix<SCALAR, 3, 1> m_position;
};

/// Represents the track of a feature in the world. Tracks can be observed by multiple frames and cameras.
template <typename FRAME_ID_T, typename TRACK_ID_T, typename POINT_T> struct FeatureTrack {
    using frame_id = FRAME_ID_T;
    using track_id = TRACK_ID_T;
    using point_type = POINT_T;

    FeatureTrack()
        : m_trackId(-1) {}
    FeatureTrack(const track_id trackId)
        : m_trackId(trackId) {}

    /// The ID of the track
    track_id m_trackId;

    /// Set of frame ID's that can observe the track
    std::unordered_set<frame_id> m_frames;

    /// 3D world point
    Point3d<point_type> m_point;
};

/// Represents the 2D observation in image and the track ID assignment.
template <typename TRACK_ID_T> struct FeaturePoint {
    using track_id = TRACK_ID_T;

    FeaturePoint()
        : m_trackId(-1) {}
    FeaturePoint(const track_id trackId, const Eigen::Vector2f& pixelPoint)
        : m_trackId(trackId) {
        m_pixelPoint.set_x(pixelPoint(0));
        m_pixelPoint.set_y(pixelPoint(1));
    }
    FeaturePoint(const track_id trackId, const feature_detectors::FeaturePoint& pixelPoint)
        : m_trackId(trackId)
        , m_pixelPoint(pixelPoint) {}
    FeaturePoint(const track_id trackId, const Eigen::Vector2f& pixelPoint, const Eigen::Vector2f& calibratedPoint)
        : m_trackId(trackId) {
        m_pixelPoint.set_x(pixelPoint(0));
        m_pixelPoint.set_y(pixelPoint(1));
        m_calibratedPoint.set_x(calibratedPoint(0));
        m_calibratedPoint.set_y(calibratedPoint(1));
    }
    FeaturePoint(
        const track_id trackId, const feature_detectors::FeaturePoint& pixelPoint, const feature_detectors::FeaturePoint& calibratedPoint)
        : m_trackId(trackId)
        , m_pixelPoint(pixelPoint)
        , m_calibratedPoint(calibratedPoint) {}

    /// Track ID of the feature point
    track_id m_trackId;

    /// 2D observation of the feature point in the image
    feature_detectors::FeaturePoint m_pixelPoint;

    /// 2D observation of the feature point in calibrated space (hnormalized - assume unit z axis)
    feature_detectors::FeaturePoint m_calibratedPoint;
};

/// Represents the camera frame and all the points that are observed.
template <typename FRAME_ID_T, typename TRACK_ID_T, typename FEATURE_POINT_T> struct Frame {
    using frame_id = FRAME_ID_T;
    using track_id = TRACK_ID_T;
    using feature_point_type = FEATURE_POINT_T;

    Frame()
        : m_frameId(-1)
        , m_timestamp(0) {}
    Frame(const frame_id frameId)
        : m_frameId(frameId)
        , m_timestamp(-1) {}
    Frame(const frame_id frameId, const double timestamp)
        : m_frameId(frameId)
        , m_timestamp(timestamp) {}

    /// The ID of the frame
    frame_id m_frameId;

    /// Timestamp in seconds
    double m_timestamp;

    /// Map of feature points indexed by track ID, i.e. get the feature point from a track ID for this frame
    std::unordered_map<track_id, FEATURE_POINT_T> m_points;

    /// Camera to IMU pose
    Pose<double> m_cameraToImuPose;

    /// IMU to World Pose
    Pose<double> m_ImuToWorldPose;
};
}
