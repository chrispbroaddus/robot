#pragma once

#include "inverse_kinematic_docker.h"
#include "packages/docking/proto/docking_station.pb.h"
#include "packages/docking/proto/vehicle_calibration.pb.h"
#include "packages/executor/executor.h"
#include "packages/teleop/proto/backend_message.pb.h"

namespace docking {

///
/// DockCommand Executor, running InverseKinematicDocker
///
class DockCommandExecutor {

public:
    DockCommandExecutor(const InverseKinematicDockerOptions ikDockingOptions,
        std::shared_ptr<docking::FiducialPoseSourceInterface> fiducialPoseSource, const std::string& fiducialStationFinderPort,
        const std::string& vehicleCalibrationFilename, const std::string& dockingStationFinderTopic);

    DockCommandExecutor(const InverseKinematicDockerOptions ikDockingOptions,
        std::shared_ptr<docking::FiducialPoseSourceInterface> fiducialPoseSource, const std::string& fiducialStationFinderPort,
        const std::string& vehicleCalibrationFilename);

    ~DockCommandExecutor() = default;
    DockCommandExecutor(const DockCommandExecutor&) = delete;
    DockCommandExecutor(const DockCommandExecutor&&) = delete;
    DockCommandExecutor& operator=(const DockCommandExecutor&) = delete;
    DockCommandExecutor& operator=(const DockCommandExecutor&&) = delete;

    void execute(std::shared_ptr<executor::Executor> executor, const teleop::DockCommand& command);

    void stop();

private:
    InverseKinematicDockerOptions m_ikDockingOptions;

    std::shared_ptr<docking::FiducialPoseSourceInterface> m_fiducialPoseSource;

    std::string m_fiducialStationFinderPort;
    std::string m_dockingStationFinderTopic;

    docking::VehicleCalibration m_vehicleCalibration;
    std::unique_ptr<docking::InverseKinematicDocker> m_docker;
};
}
