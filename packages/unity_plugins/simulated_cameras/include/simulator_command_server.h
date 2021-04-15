#pragma once

#include "camera_output_addresses.h"
#include "packages/hal/include/string_utils.h"
#include "packages/hal/proto/simulator_command_envelope.pb.h"
#include "packages/hal/proto/simulator_command_response.pb.h"
#include "simulator_network_interop.h"

#include <atomic>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <zmq.hpp>

namespace unity_plugins {

class SimulatorCommandBase;

///
/// @brief The command server receives commands from a client and calls back to Unity on the main thread to act on these commands
///
class SimulatorCommandServer {

public:
    ///
    /// @brief create a command server
    ///
    SimulatorCommandServer(std::shared_ptr<zmq::context_t> context, const SimulatorNetworkManagerSettings& networkSettings,
        const std::map<hal::CameraId, CameraOutputAddresses>& cameraAddresses);
    ~SimulatorCommandServer();

    ///
    /// @brief This has to be called to run the queued commands.
    /// This will be called by Unity on the main thread during its update step
    ///
    void processCommandQueue();

private:
    void listen(std::shared_ptr<zmq::context_t> context);
    void processCameraCommand(hal::SimulatorCommandResponse& response, const hal::SimulatorCameraCommand& cameraCommand);
    void processResetCommand(hal::SimulatorCommandResponse& response, const hal::SimulatorResetCommand& resetCommand);
    void processStatsCommand(hal::SimulatorCommandResponse& response, const hal::SimulatorStatsCommand& statsCommand);
    void enqueueCommand(std::shared_ptr<SimulatorCommandBase> cmd);

    const SimulatorNetworkManagerSettings& m_settings;
    const std::map<hal::CameraId, CameraOutputAddresses>& m_cameraAddresses;

    std::atomic<bool> m_stop;
    std::thread m_thread;

    std::mutex m_queueMutex;
    std::queue<std::shared_ptr<SimulatorCommandBase> > m_commandQueue;
};
} // namespace unity_plugins
