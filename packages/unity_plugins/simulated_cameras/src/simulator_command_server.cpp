#include "packages/unity_plugins/simulated_cameras/include/simulator_command_server.h"
#include "glog/logging.h"
#include "packages/net/include/zmq_rep_server.h"
#include "packages/unity_plugins/simulated_cameras/include/simulator_publisher_topics.h"

namespace unity_plugins {

class SimulatorCommandBase {
public:
    SimulatorCommandBase(){};
    virtual ~SimulatorCommandBase() = default;

    virtual void Run() = 0;
};

template <typename T, typename C> class SimulatorCommand : public SimulatorCommandBase {
public:
    SimulatorCommand(const T& data, C callback) {
        m_data = data;
        m_callback = callback;
    }
    ~SimulatorCommand() = default;

    void Run() { m_callback(m_data); }

private:
    T m_data;
    C m_callback;
};

} // namepsace unity_plugins

using namespace unity_plugins;

SimulatorCommandServer::SimulatorCommandServer(std::shared_ptr<zmq::context_t> context,
    const SimulatorNetworkManagerSettings& networkSettings, const std::map<hal::CameraId, CameraOutputAddresses>& cameraAddresses)
    : m_settings(networkSettings)
    , m_cameraAddresses(cameraAddresses)
    , m_stop(false)
    , m_thread(&SimulatorCommandServer::listen, this, context) {}

SimulatorCommandServer::~SimulatorCommandServer() {
    m_stop = true;
    m_thread.join();
}

void SimulatorCommandServer::listen(std::shared_ptr<zmq::context_t> context) {

    std::string addr = std::string(m_settings.commandServer.serverAddress);
    net::ZMQProtobufRepServer<hal::SimulatorCommandEnvelope, hal::SimulatorCommandResponse> server(
        *context, addr, m_settings.network.zmqLingerTimeMs, m_settings.network.zmqSendRecvTimeoutMs);

    hal::SimulatorCommandEnvelope controlMsg;

    while (!m_stop) {
        //  Wait for next request from client
        if (server.recv(controlMsg)) {
            hal::SimulatorCommandResponse response;

            if (controlMsg.has_cameracommand()) {
                processCameraCommand(response, controlMsg.cameracommand());
            } else if (controlMsg.has_resetcommand()) {
                processResetCommand(response, controlMsg.resetcommand());
            } else if (controlMsg.has_statscommand()) {
                processStatsCommand(response, controlMsg.statscommand());
            }

            server.send(response);
        }
    }
}

hal::SimulatorCameraOutput* populateCameraOutput(const std::string& address) {

    auto output = new hal::SimulatorCameraOutput();
    output->set_address(address);
    output->set_topic(CAMERA_OUTPUT_TOPIC);

    return output;
}

void SimulatorCommandServer::processCameraCommand(
    hal::SimulatorCommandResponse& response, const hal::SimulatorCameraCommand& cameraCommand) {

    bool success = false;
    if (m_settings.commandServer.cameraConfigCallback) {
        CameraConfigurationCommand cameraConfig;
        cameraConfig.cameraId = cameraCommand.cameraid();
        cameraConfig.enable = cameraCommand.enable() ? 1 : 0;
        cameraConfig.enableDepth = cameraCommand.enabledepth() ? 1 : 0;
        cameraConfig.enablePointcloud = cameraCommand.enablexyz() ? 1 : 0;

        auto cmd = std::make_shared<SimulatorCommand<CameraConfigurationCommand, CameraConfigurationCallback> >(
            cameraConfig, m_settings.commandServer.cameraConfigCallback);
        enqueueCommand(cmd);
        success = true;
    }

    response.set_status(success ? hal::StatusCode::StatusSuccess : hal::StatusCode::StatusFailure);

    auto cameraResponse = new hal::SimulatorCameraResponse();

    auto& addresses = m_cameraAddresses.at(cameraCommand.cameraid());

    auto imageOutput = populateCameraOutput(addresses.image);
    auto depthOutput = populateCameraOutput(addresses.depth);
    auto pointcloudOutput = populateCameraOutput(addresses.pointcloud);

    cameraResponse->set_allocated_image(imageOutput);
    cameraResponse->set_allocated_depth(depthOutput);
    cameraResponse->set_allocated_xyz(pointcloudOutput);

    response.set_allocated_cameraresponse(cameraResponse);
}

void SimulatorCommandServer::processResetCommand(hal::SimulatorCommandResponse& response, const hal::SimulatorResetCommand& resetCommand) {

    bool success = false;

    if (m_settings.commandServer.resetCallback) {
        SimulatorResetCommand reset;
        reset.robot = resetCommand.resetrobot() ? 1 : 0;
        reset.world = resetCommand.resetworld() ? 1 : 0;
        reset.streams = resetCommand.resetstreams() ? 1 : 0;

        auto cmd = std::make_shared<SimulatorCommand<SimulatorResetCommand, ResetCallback> >(reset, m_settings.commandServer.resetCallback);
        enqueueCommand(cmd);
        success = true;
    }

    response.set_status(success ? hal::StatusCode::StatusSuccess : hal::StatusCode::StatusFailure);
    auto resetResponse = new hal::SimulatorResetResponse();
    response.set_allocated_resetresponse(resetResponse);
}

void SimulatorCommandServer::processStatsCommand(hal::SimulatorCommandResponse& response, const hal::SimulatorStatsCommand& statsCommand) {
    bool success = false;

    if (m_settings.commandServer.statsCallback) {
        StatsCommand stats;
        stats.enable = statsCommand.enable() ? 1 : 0;
        auto cmd = std::make_shared<SimulatorCommand<StatsCommand, StatsCallback> >(stats, m_settings.commandServer.statsCallback);
        enqueueCommand(cmd);
        success = true;
    }

    response.set_status(success ? hal::StatusCode::StatusSuccess : hal::StatusCode::StatusFailure);
    auto statsResponse = new hal::SimulatorStatsResponse();

    statsResponse->set_address(m_settings.statsPublisher.statsAddress);
    statsResponse->set_topic(STATS_TOPIC);

    response.set_allocated_statsresponse(statsResponse);
}

void SimulatorCommandServer::enqueueCommand(std::shared_ptr<SimulatorCommandBase> cmd) {
    std::lock_guard<std::mutex> guard(m_queueMutex);
    m_commandQueue.push(cmd);
}

void SimulatorCommandServer::processCommandQueue() {
    std::lock_guard<std::mutex> guard(m_queueMutex);

    while (!m_commandQueue.empty()) {
        auto cmd = m_commandQueue.front();
        m_commandQueue.pop();

        cmd->Run();
    }
}
