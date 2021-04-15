#include "dock_command_executor.h"

#include "glog/logging.h"
#include "packages/calibration/proto/system_calibration.pb.h"

namespace docking {

DockCommandExecutor::DockCommandExecutor(const InverseKinematicDockerOptions ikDockingOptions,
    std::shared_ptr<docking::FiducialPoseSourceInterface> fiducialPoseSource, const std::string& fiducialStationFinderPort,
    const std::string& vehicleCalibrationFilename)
    : DockCommandExecutor::DockCommandExecutor(
          ikDockingOptions, fiducialPoseSource, fiducialStationFinderPort, vehicleCalibrationFilename, "docking_observation") {}

DockCommandExecutor::DockCommandExecutor(const InverseKinematicDockerOptions ikDockingOptions,
    std::shared_ptr<docking::FiducialPoseSourceInterface> fiducialPoseSource, const std::string& fiducialStationFinderPort,
    const std::string& vehicleCalibrationFilename, const std::string& dockingStationFinderTopic)
    : m_ikDockingOptions(ikDockingOptions)
    , m_fiducialPoseSource(fiducialPoseSource)
    , m_fiducialStationFinderPort(fiducialStationFinderPort)
    , m_dockingStationFinderTopic(dockingStationFinderTopic) {

    std::ifstream vehicleCalibrationFile(vehicleCalibrationFilename);
    std::stringstream vehicleCalibrationJsonBuffer;
    vehicleCalibrationJsonBuffer << vehicleCalibrationFile.rdbuf();
    google::protobuf::util::JsonStringToMessage(vehicleCalibrationJsonBuffer.str(), &m_vehicleCalibration);

    CHECK(m_vehicleCalibration.distbetweenwheel() != 0) << "The data loaded from " + vehicleCalibrationFilename + " has wrong value.";
}

void DockCommandExecutor::execute(std::shared_ptr<executor::Executor> executor, const teleop::DockCommand& command) {

    zmq::context_t context(1);
    net::ZMQProtobufSubscriber<DockingStationList> subscriber(
        context, "tcp://localhost:" + m_fiducialStationFinderPort, m_dockingStationFinderTopic, 1);

    DockingStationList list;
    if (subscriber.poll(std::chrono::milliseconds(100))) {
        subscriber.recv(list);

        LOG(INFO) << __PRETTY_FUNCTION__ << " ... observed docking_stations_size() : " << list.docking_stations_size();
        int targetStationIndex = -1;
        for (int i = 0; i < list.docking_stations_size(); i++) {
            if (list.docking_stations(i).station_id() == command.station_id()) {
                targetStationIndex = i;
            }
        }

        if (targetStationIndex >= 0) {
            LOG(INFO) << "Starting InverseKinematicDocker...";
            m_docker.reset(new docking::InverseKinematicDocker(
                executor, m_fiducialPoseSource, m_vehicleCalibration, list.docking_stations(targetStationIndex), m_ikDockingOptions));
        } else {
            LOG(INFO) << "WARNING : Cannot find the target station from the scene.";
        }
    }
}

void DockCommandExecutor::stop() { m_docker.reset(); }
}
