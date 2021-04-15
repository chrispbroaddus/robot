#include "fiducial_station_finder.h"
#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/planning/utils.h"

using namespace docking;

FiducialStationFinder::FiducialStationFinder(std::shared_ptr<docking::FiducialPoseSourceInterface> fiducialPoseSource,
    const std::string& pubPort, const int highWaterMark, const int lingerPeriodInMilliseconds, const int publishIntervalInMilliseconds,
    const std::string& topic, const std::string& stationListJsonFilename)
    : m_publishIntervalInMilliseconds(publishIntervalInMilliseconds)
    , m_topic(topic)
    , m_stop(false)
    , m_thread(&FiducialStationFinder::publish, this, "tcp://*:" + pubPort, highWaterMark, lingerPeriodInMilliseconds, std::ref(m_stop))
    , m_fiducialPoseSubscriber(fiducialPoseSource) {
    std::ifstream file(stationListJsonFilename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    google::protobuf::util::JsonStringToMessage(buffer.str(), &m_stationList);
}

FiducialStationFinder::~FiducialStationFinder() {
    m_stop = true;
    m_thread.join();
}

void FiducialStationFinder::publish(
    const std::string& serverAddress, const int highWaterMark, const int lingerPeriodInMilliseconds, std::atomic_bool& stop) {
    zmq::context_t context(1);
    net::ZMQProtobufPublisher<docking::DockingStationList> publisher(context, serverAddress, highWaterMark, lingerPeriodInMilliseconds);
    while (!stop) {

        /// Search from the image
        docking::DockingStationList feasibleStationList;
        for (int i = 0; i < m_stationList.docking_stations_size(); i++) {
            std::vector<calibration::CoordinateTransformation> posesCameraWrtFiducial;
            if (m_fiducialPoseSubscriber
                && m_fiducialPoseSubscriber->readPoses(posesCameraWrtFiducial, m_stationList.docking_stations(i))) {
                auto& station = m_stationList.docking_stations(i);
                bool existMatch = false;
                for (int j = 0; j < station.fiducials_size(); j++) {
                    const auto& name = station.fiducials(j).transformation().sourcecoordinateframe().device().name();
                    const auto& serial = station.fiducials(j).transformation().sourcecoordinateframe().device().serialnumber();
                    for (int k = 0; k < (int)posesCameraWrtFiducial.size(); k++) {

                        VLOG(2) << __FUNCTION__ << "Name : Detected: " << posesCameraWrtFiducial[k].sourcecoordinateframe().device().name()
                                << " vs. "
                                << "database : " << name;
                        VLOG(2) << __FUNCTION__
                                << "Serial : Detected: " << posesCameraWrtFiducial[k].sourcecoordinateframe().device().serialnumber()
                                << " vs. "
                                << "database : " << serial;

                        /// Compare the apriltag name (family) & serialnumber (id) between
                        /// station metadata in the database vs detected metadata from the apriltag detector.
                        if (name == posesCameraWrtFiducial[k].sourcecoordinateframe().device().name()
                            && serial == posesCameraWrtFiducial[k].sourcecoordinateframe().device().serialnumber()) {
                            existMatch = true;
                            break;
                        }
                    }
                    if (existMatch) {
                        break;
                    }
                }
                if (existMatch) {
                    auto newStation = feasibleStationList.add_docking_stations();
                    newStation->ParseFromString(station.SerializeAsString());
                }
            }
        }
        publisher.send(feasibleStationList, m_topic);
        VLOG(2) << __FUNCTION__
                << " ... sent out the docking observation, stations.size() : " << feasibleStationList.docking_stations_size();
        std::this_thread::sleep_for(std::chrono::milliseconds(m_publishIntervalInMilliseconds));
    }
}
