#include "packages/planning/utils.h"

#include <chrono>
#include <fstream>
#include <unistd.h>

#include "packages/core/include/chrono.h"
#include "packages/planning/vcu_trajectory_check.h"

using core::chrono::gps::wallClockInNanoseconds;

namespace planning {

bool sample(const TrajectoryElement& element, std::vector<core::Point2d>& points, float increment) {
    CHECK(element.arclength() > 0);
    CHECK(element.curvature() != 0);
    float radius = 1 / std::abs(element.curvature());
    CHECK(radius > 0);
    auto theta = increment / radius;
    CHECK(theta > 0);
    float distance = 0;
    float angle = 0;
    float _x, _y;
    int sign_curvature = element.curvature() > 0 ? 1 : -1;
    int sign_velocity = element.linear_velocity() > 0 ? 1 : -1;
    points.clear();

    while (distance < element.arclength()) {
        _x = radius * std::cos(angle);
        _y = radius * std::sin(angle);
        distance += radius * theta;
        angle += theta;
        wykobi::point2d<float> _point;
        _point.x = -1 * sign_curvature * (_x - radius);
        _point.y = sign_velocity * _y;
        core::Point2d point;
        wykobiToZippy(_point, point);
        points.emplace_back(point);
    }
    return (points.size() > 0);
}

std::ostream& operator<<(std::ostream& stream, const Path& path) {
    for (int i = 0; i < path.elements_size(); ++i) {
        const auto element = path.elements(i);
        stream << element.transform().translationx() << " " << element.transform().translationx() << " "
               << element.transform().translationz() << std::endl;
    }
    return stream;
}

void toAbsoluteTime(const std::chrono::nanoseconds& system, const std::chrono::nanoseconds& planning_tolerance, Trajectory& trajectory) {
    // Note: commands need to be in the future for VCU interop
    auto current = system + planning_tolerance;

    for (int i = 0; i < trajectory.elements_size(); ++i) {
        core::SystemTimestamp target;
        target.set_nanos(current.count());
        *trajectory.mutable_elements(i)->mutable_absolute_time() = target;
        auto next = std::chrono::nanoseconds(trajectory.elements(i).relative_time().nanos());
        current += next;
    }
}

void relativeTrajectoryToAbsolute(const Trajectory& relative, std::chrono::nanoseconds planningTolerance, Trajectory& absolute) {
    absolute = relative;
    CHECK(absolute.elements_size() > 0);
    auto begin = core::chrono::gps::wallClockInNanoseconds();
    toAbsoluteTime(begin, planningTolerance, absolute);
}

void plannerTrajectoryToVCUTrajectory(const Trajectory& plannerTrajectory, hal::VCUTrajectoryCommand& halTrajectory) {
    halTrajectory.clear_segments();
    for (int i = 0; i < plannerTrajectory.elements_size(); ++i) {
        auto segment = halTrajectory.add_segments();
        *segment->mutable_targetstarttime() = plannerTrajectory.elements(i).absolute_time();
        hal::VCUArcDriveSegment arcDriveSegment;
        arcDriveSegment.set_curvatureinversemeters(plannerTrajectory.elements(i).curvature());
        arcDriveSegment.set_linearvelocitymeterspersecond(plannerTrajectory.elements(i).linear_velocity());
        *segment->mutable_arcdrive() = arcDriveSegment;
    }
    CHECK(plannerTrajectory.elements_size() == halTrajectory.segments_size());
    CHECK(static_cast<size_t>(halTrajectory.segments_size()) < PlannerConstants<float>::kMaxEntriesPerCommand);
}

void zippyToOpenGL(const core::Point2d& input, core::Point2d& output) {
    Sophus::SE3d input_(Eigen::Quaterniond{ 1, 0, 0, 0 }, Eigen::Vector3d{ 0, 0, 0 });
    Sophus::SE3d output_ = input_;
    zippyToOpenGL(input_, output_);
    Eigen::Vector3d point{ input.x(), input.y(), 0.0 };
    Eigen::Vector3d result = output_ * point;
    output.set_x(result[0]);
    output.set_y(result[1]);
}

ZMQComms::ZMQComms(const std::string& address, int port, bool require_acknowledge)
    : m_require_acknowledge(require_acknowledge)
    , m_sequence_number(1) {
    std::stringstream fullyQualifiedAddress;
    fullyQualifiedAddress << address << ":" << port;
    constexpr int kLingerMs = 500;
    constexpr int kDefaultTimeoutMs = 1000;
    m_comms.reset(new comms_t(m_context, fullyQualifiedAddress.str(), kLingerMs, kDefaultTimeoutMs));
    m_commsLog.reset(new BaseLogger<hal::VCUCommandEnvelope>("zmqcomms.log"));
}

bool ZMQComms::send(const Trajectory& trajectory, hal::VCUCommandResponse* optional_response) {
    LOG(INFO) << trajectory.DebugString();
    std::lock_guard<std::mutex> lock(m_guard);
    hal::VCUTrajectoryCommand vcuTrajectory;
    plannerTrajectoryToVCUTrajectory(trajectory, vcuTrajectory);
    hal::VCUCommandEnvelope envelope;
    envelope.set_sequencenumber(m_sequence_number++);
    *envelope.mutable_trajectorycommand() = vcuTrajectory;
    m_commsLog->add(envelope);

    int errorCode = 0;
    if (!commandValidate(envelope, errorCode)) {
        LOG(ERROR) << "Tried to send invalid envelope!";
        LOG(ERROR) << envelope.DebugString();
        LOG(ERROR) << "Code: " << errorCode;
        LOG(FATAL) << "Cannot continue";
    }

    CHECK(m_comms->send(envelope)) << "Failed to send envelope!";
    hal::VCUCommandResponse response;
    if (m_require_acknowledge) {
        CHECK(m_comms->recv(response));
        if (optional_response != nullptr) {
            *optional_response = response;
        }
        bool result = (response.disposition() == hal::VCUCommandDisposition::CommandAccepted);
        if (!result) {
            LOG(ERROR) << "Receipt failure: ";
            LOG(ERROR) << response.DebugString();
            LOG(ERROR) << VCUCommandDisposition_Name(response.disposition());
            LOG(ERROR) << envelope.DebugString();
        }
        return result;
    } else {
        return true;
    }
}

} // planning
