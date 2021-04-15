#include "fiducial_pose_subscriber.h"
#include "glog/logging.h"
#include "packages/core/include/chrono.h"

namespace docking {

FiducialPoseSubscriber::FiducialPoseSubscriber(const std::string& addr, const std::string& topic)
    : m_stop(false)
    , m_thread(&FiducialPoseSubscriber::listen, this, addr, topic, std::ref(m_stop)) {}

FiducialPoseSubscriber::~FiducialPoseSubscriber() {
    m_stop = true;
    m_thread.join();
}

bool FiducialPoseSubscriber::readPose(
    std::vector<calibration::CoordinateTransformation>& poses, const docking::DockingStation& dockingStation) {
    std::lock_guard<std::mutex> guard(m_mutex);
    if (!m_poses.transformations_size() == 0)
        return false;

    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds().count();
    if (gpsTimestamp - m_poses.system_timestamp().nanos() < 100 * 1e6) // 100 ms
    {

        VLOG(1) << __FUNCTION__ << " ... m_poses.transformations_size() : " << m_poses.transformations_size();
        for (int i = 0; i < m_poses.transformations_size(); i++) {
            bool foundFiducialOnDocker = false;
            auto pose = m_poses.transformations(i);
            for (int j = 0; j < dockingStation.fiducials_size(); j++) {
                VLOG(1) << __FUNCTION__
                        << " ... ... pose.sourcecoordinateframe().device().name() : " << pose.sourcecoordinateframe().device().name();
                VLOG(1) << __FUNCTION__
                        << " ... ... dockingStation.fiducials(j).transformation().sourcecoordinateframe().device().name() : "
                        << dockingStation.fiducials(j).transformation().sourcecoordinateframe().device().name();
                if (pose.sourcecoordinateframe().device().name()
                    == dockingStation.fiducials(j).transformation().sourcecoordinateframe().device().name()) {
                    foundFiducialOnDocker = true;
                }
            }

            if (foundFiducialOnDocker) {
                poses.push_back(pose);
            }
        }
        m_poses.clear_transformations();
        m_poses.clear_system_timestamp();

        return true;
    } else {
        return false;
    }
}

void FiducialPoseSubscriber::listen(const std::string& addr, const std::string& topic, std::atomic_bool& stop) {

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<perception::FiducialPoses> subscriber(context, addr, topic, 1);

    while (!stop) {
        if (subscriber.poll(std::chrono::microseconds(10))) {
            perception::FiducialPoses poses;
            if (subscriber.recv(poses) && poses.transformations_size() > 0) {
                std::lock_guard<std::mutex> guard(m_mutex);
                std::swap(m_poses, poses);

                VLOG(1) << __FUNCTION__ << "received an apriltag poses. Cardinality : " << m_poses.transformations_size()
                        << ", timestamp: " << m_poses.system_timestamp().nanos();
            } // if (subscriber.recv(envelope))

        } // if (subscriber.poll(std::chrono::microseconds(10)))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } // while(!stop)
}
}
