#pragma once

#include "apriltag_fiducial_pose_source.h"
#include "google/protobuf/util/json_util.h"
#include "ground_truth_fiducial_pose_source.h"
#include "packages/net/include/zmq_rep_server.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "station_finder_interface.h"
#include <atomic>
#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>

namespace docking {

///
/// Find the feasible local stations and publish, from detected fiducials metadata
///
class FiducialStationFinder : public StationFinderInterface {
public:
    FiducialStationFinder(std::shared_ptr<docking::FiducialPoseSourceInterface> fiducialPoseSource, const std::string& pubPort,
        const int highWaterMark, const int lingerPeriodInMilliseconds, const int publishIntervalInMilliseconds, const std::string& topic,
        const std::string& stationListJsonFilename);

    FiducialStationFinder(std::shared_ptr<docking::FiducialPoseSourceInterface> fiducialPoseSource, const std::string& pubPort,
        const int highWaterMark, const int lingerPeriodInMilliseconds, const int publishIntervalInMilliseconds,
        const std::string& stationListJsonFilename)
        : FiducialStationFinder(fiducialPoseSource, pubPort, highWaterMark, lingerPeriodInMilliseconds, publishIntervalInMilliseconds,
              "docking_observation", stationListJsonFilename) {}

    FiducialStationFinder(const FiducialStationFinder& obj) = delete;
    FiducialStationFinder& operator=(FiducialStationFinder other) = delete;

    ~FiducialStationFinder();

private:
    ///
    /// @brief Find the available docking station (where a docking command can be executed)
    ///
    void publish(const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds, std::atomic_bool& stop);

    int m_publishIntervalInMilliseconds;
    std::string m_topic;
    docking::DockingStationList m_stationList;
    std::atomic_bool m_stop;
    std::thread m_thread;
    std::shared_ptr<docking::FiducialPoseSourceInterface> m_fiducialPoseSubscriber;
};
}
