#include "ground_truth_fiducial_pose_source.h"
#include "glog/logging.h"
#include "packages/core/include/chrono.h"

namespace docking {

GroundTruthFiducialPoseSource::GroundTruthFiducialPoseSource(const std::string& addr, const std::string& topic)
    : m_stop(false)
    , m_thread(&GroundTruthFiducialPoseSource::listen, this, addr, topic, std::ref(m_stop)) {}

GroundTruthFiducialPoseSource::~GroundTruthFiducialPoseSource() {
    m_stop = true;
    m_thread.join();
}

bool GroundTruthFiducialPoseSource::readPoses(
    std::vector<calibration::CoordinateTransformation>& poses, const docking::DockingStation& dockingStation) {
    std::lock_guard<std::mutex> guard(m_mutex);
    if (m_poses.transformations_size() == 0)
        return false;

    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds().count();
    if (gpsTimestamp - m_poses.system_timestamp().nanos() < 100 * 1e6) // 100 ms
    {

        VLOG(1) << __FUNCTION__ << " ... m_poses.transformations_size() : " << m_poses.transformations_size();
        for (int i = 0; i < m_poses.transformations_size(); i++) {
            bool foundFiducialOnDocker = false;
            auto pose = m_poses.transformations(i);
            for (int j = 0; j < dockingStation.fiducials_size(); j++) {
                VLOG(2) << __FUNCTION__ << " ... ... source : " << pose.sourcecoordinateframe().device().name();
                VLOG(2) << __FUNCTION__
                        << " ... ... target : " << dockingStation.fiducials(j).transformation().sourcecoordinateframe().device().name();
                if (pose.sourcecoordinateframe().device().name()
                        == dockingStation.fiducials(j).transformation().sourcecoordinateframe().device().name()
                    && pose.sourcecoordinateframe().device().serialnumber()
                        == dockingStation.fiducials(j).transformation().sourcecoordinateframe().device().serialnumber()) {
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
        LOG(WARNING) << "Fiducial poses are outdated.";
        return false;
    }
}

void GroundTruthFiducialPoseSource::listen(const std::string& addr, const std::string& topic, std::atomic_bool& stop) {

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<perception::FiducialPoses> subscriber(context, addr, topic, 1);

    while (!stop) {
        if (subscriber.poll(std::chrono::milliseconds(1))) {
            perception::FiducialPoses poses;
            if (subscriber.recv(poses) && poses.transformations_size() > 0) {
                std::lock_guard<std::mutex> guard(m_mutex);
                std::swap(m_poses, poses);

            } // if (subscriber.recv(envelope))

        } // if (subscriber.poll(std::chrono::microseconds(10)))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } // while(!stop)
}
}
