#pragma once

#include <atomic>
#include <chrono>
#include <cmath>
#include <deque>
#include <fstream>
#include <mutex>
#include <thread>

#include "glog/logging.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/util/json_util.h"
#include "sophus/se3.hpp"
#include "wykobi/wykobi.hpp"

#include "packages/calibration/proto/coordinate_transformation.pb.h"
#include "packages/core/include/chrono.h"
#include "packages/core/proto/geometry.pb.h"
#include "packages/hal/proto/vcu_command_response.pb.h"
#include "packages/net/include/zmq_req_client.h"
#include "packages/planning/logging.h"
#include "packages/planning/proto/path.pb.h"
#include "packages/planning/proto/trajectory.pb.h"
#include "packages/planning/proto_helpers.h"

namespace planning {

template <typename T> struct PhysicalConstants { static constexpr T kSecondsToNanoseconds = 1000000000; };

template <typename T> constexpr T PhysicalConstants<T>::kSecondsToNanoseconds;

/**
 * @brief Convert a POD-type to a duration
 *
 * @tparam T      POD-type
 * @param input   Value to be converted to duration
 *
 * @return        Duration
 */
template <typename T> core::Duration toDuration(T input);

/**
 * @brief Convert a Trajectory sequence, consisting of relative times, to an
 *        equivalent Trajectory with absolute timestamps.
 *
 *        This is needed because planning is done relatively, but the VCU only
 *        operates on absolute time.
 *
 * @param[in] system                Current system time
 * @param[in] offset                Clock-skew offset to apply
 * @param[in] planning_tolerance    Planning tolerance (to account for
 *                                  serialization, etc.)
 * @param[out] trajectory           Trajectory with absolute times
 */
void toAbsoluteTime(const std::chrono::nanoseconds& system, const std::chrono::nanoseconds& offset,
    const std::chrono::nanoseconds& planning_tolerance, Trajectory& trajectory);

/**
 * @brief Generate point samples along a given arc trajectory element
 *
 * @param[in] element   TrajectoryElement specifying arc parameters
 * @param[out] points   Sampled points
 * @param[in] increment Arc-distance to sample
 *
 * @return Path was successfully sampled
 */
bool sample(const TrajectoryElement& element, std::vector<core::Point2d>& points, float increment = 1.0);

/**
 * @brief Methods to convert between Wykobi and Zippy co-ordinate systems.
 *        Wykobi: Matlab-like, (x-right, y-forward, z-up)
 *        Zippy: NASA-like, (x-forward, y-right, z-down)
 *
 * @tparam T            Numeric type
 * @param[in] input     Wykobi input
 * @param[out] output   Core output
 */
template <typename T> void wykobiToZippy(const wykobi::point2d<T>& input, core::Point2d& output);

template <typename T> void wykobiToZippy(const wykobi::point3d<T>& input, core::Point3d& output);

template <typename T> void zippyToWykobi(const core::Point2d& input, wykobi::point2d<T>& output);

template <typename T> void zippyToWykobi(const core::Point3d& input, wykobi::point3d<T>& output);

template <typename T> void zippyToWykobi(const Sophus::SE3<T>& input, wykobi::point3d<T>& output);

template <typename T> void zippyToWykobi(const Sophus::SE3<T>& input, wykobi::point2d<T>& output);

/**
 * @brief Convert between Sophus geometry types, and the protobuf
 * representation.
 *
 * @tparam T        Numeric type
 * @param[in] in    Geometry
 * @param[out] out  Protobuf
 */
template <typename T> void poseToProto(const Sophus::SE3<T>& in, calibration::CoordinateTransformation& out);

template <typename T> void protoToPose(const calibration::CoordinateTransformation& in, Sophus::SE3<T>& out);

/**
 * @brief Convert to/from Zippy/SBPL frames
 *        SBPL: Matlab-like, (x-right, y-forward, z-up)
 *        Zippy: NASA-like, (x-forward, y-right, z-down)
 *
 * @tparam T            Numeric type
 * @param[in] input     Geometry
 * @param[out] output   Geometry
 */
template <typename T> void zippyToSBPL(const Sophus::SE3<T>& input, Sophus::SE3<T>& output);

template <typename T> void SBPLToZippy(const Sophus::SE3<T>& input, Sophus::SE3<T>& output);

/**
 * @brief Convert to/from Zippy/OpenGL frames
 *
 * @tparam T            Numeric type
 * @param[in] input     Geometry
 * @param[out] output   Geometry
 */
template <typename T> void zippyToOpenGL(const Sophus::SE3<T>& input, Sophus::SE3<T>& output);

template <typename T> void zippyToOpenGL(const Eigen::Matrix<T, 4, 1>& input, Eigen::Matrix<T, 4, 1>& output);

template <typename T> void zippyToOpenGL(const Eigen::Matrix<T, 3, 1>& input, Eigen::Matrix<T, 3, 1>& output);

void zippyToOpenGL(const core::Point2d& input, core::Point2d& output);

/**
 * @brief Convert to/from XYZRPQ/SE3 representation
 *
 * @tparam T        Numeric type
 * @param[in] x     X (m)
 * @param[in] y     Y (m)
 * @param[in] z     Z (m)
 * @param[in] r     Roll (rad)
 * @param[in] p     Pitch (rad)
 * @param[in] q     Yaw (rad)
 * @param[out] pose SE3 representation
 */
template <typename T> void elementsToPose(T x, T y, T z, T r, T p, T q, Sophus::SE3<T>& pose);

template <typename T> void poseToElements(T& x, T& y, T& z, T& r, T& p, T& q, const Sophus::SE3<T>& pose);

/**
 * @brief Apply SE3 transform to a Path sequence
 *
 * @tparam T            Numeric type
 * @param[in] transform SE3 transform to be applied
 * @param[in/out] path  Path input & output
 */
template <typename T> void transform(const Sophus::SE3<T> transform, Path& path);

/**
 * @brief Apply SE3 transform to a Trajectory sequence
 *
 * @tparam T                    Numeric type
 * @param[in] transform         SE3 transform to be applied
 * @param[in/out] trajectory    Trajectory input & output
 */
template <typename T> void transform(const Sophus::SE3<T> transform, Trajectory& trajectory);

/**
 * @brief Convert a Trajectory described by relative timestamps to one described
 *        by absolute timestamps.
 *
 * @param[in] relative                  Trajectory described by relative
 *                                      timestamps
 * @param[in] estimatedTimingOffset     The timing offset to be applied
 * @param[out] absolute                 Trajectory described by absolute
 *                                      timestamps
 */
void relativeTrajectoryToAbsolute(const Trajectory& relative, std::chrono::nanoseconds estimatedTimingOffset, Trajectory& absolute);

void plannerTrajectoryToVCUTrajectory(const Trajectory& planner, hal::VCUTrajectoryCommand& hal);

/**
 * @brief Comms interface for all planning code.
 */
class Comms {
public:
    /**
     * @brief Send a trajectory to the VCU (real or simulated). Optionally return
     *        the VCU response if requested.
     *
     * @param[in] trajectory          Trajectory (proto/trajectory.proto) to send
     * @param[out] optional_response  VCU response (if requested)
     *
     * @return Command was sent to the VCU, and the response disposition was
     *         ACCEPTED
     */
    virtual bool send(const Trajectory& trajectory, hal::VCUCommandResponse* optional_response = nullptr) = 0;

    virtual ~Comms() = default;
};

/**
 * @brief ZMQ-specific communications class
 */
class ZMQComms : public Comms {
    typedef net::ZMQProtobufReqClient<hal::VCUCommandEnvelope, hal::VCUCommandResponse> comms_t;

public:
    /**
     * @brief Construct a ZMQComms instance. By default, do not require
     *        acknowledgement from the target VCU (real or synthetic).
     *
     * @param[in] address               Protocol + address  (tcp://localhost)
     * @param[in] port                  Port number to connect to
     * @param[in] require_acknowledge   Whether to require acknowledgement from
     *                                  the VCU. If this is *not* required,
     *                                  _send_ will always return true if the
     *                                  trajectory is valid.
     */
    ZMQComms(const std::string& address, int port, bool require_acknowledge = false);

    bool send(const Trajectory& trajectory, hal::VCUCommandResponse* optional_response = nullptr) override;

private:
    zmq::context_t m_context = zmq::context_t(1);
    bool m_require_acknowledge;
    int m_sequence_number;
    std::mutex m_guard;
    std::unique_ptr<comms_t> m_comms;
    std::unique_ptr<BaseLogger<hal::VCUCommandEnvelope> > m_commsLog;
};

template <class T> void identity(Sophus::SE3<T>& element);

template <class T> bool checkForNumeric(const Sophus::SE3<T>& element);

} // planning

#include "utils.hpp"
