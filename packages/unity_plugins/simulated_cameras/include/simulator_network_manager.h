#pragma once

#include "camera_output_addresses.h"
#include "camera_publisher_group.h"
#include "packages/hal/proto/camera_id.pb.h"
#include "simulator_command_server.h"
#include "simulator_network_interop.h"
#include "simulator_stats_publisher.h"
#include <map>
#include <memory>
#include <unordered_map>
#include <zmq.hpp>

namespace unity_plugins {

///
/// @brief The network manager manages the network connections, command server, camera publishing
///
class SimulatorNetworkManager {
public:
    ///
    /// @brief create the manager
    ///
    /// @params[in] settings : settings for the networking components
    ///
    SimulatorNetworkManager(const SimulatorNetworkManagerSettings& settings);
    ~SimulatorNetworkManager() = default;

    ///
    /// @brief call this regularly on the main thread to process things that need to run on the main thread (eg commands)
    /// This will be called by Unity in its update step
    ///
    void update();

    ///
    /// @brief get the camera publisher group
    ///
    std::shared_ptr<CameraPublisherGroup> cameraPublisherGroup() { return m_cameraPublisherGroup; }

    ///
    /// @brief get the stats publisher
    ///
    SimulatorStatsPublisher& statsPublisher() { return *m_statsPublisher; }

private:
    void createCameraAddressMap(const SimulatorCameraPublisherSettings& settings);

private:
    SimulatorNetworkManagerSettings m_settings;
    std::shared_ptr<zmq::context_t> m_context;
    std::unique_ptr<SimulatorCommandServer> m_simulatorCommandServer;
    std::map<hal::CameraId, CameraOutputAddresses> m_cameraAddresses;
    std::shared_ptr<CameraPublisherGroup> m_cameraPublisherGroup;
    std::unique_ptr<SimulatorStatsPublisher> m_statsPublisher;
};
} // namespace unity_plugins
