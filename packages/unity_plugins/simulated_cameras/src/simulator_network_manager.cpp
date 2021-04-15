#include "packages/unity_plugins/simulated_cameras/include/simulator_network_manager.h"
#include "glog/logging.h"

using namespace unity_plugins;

SimulatorNetworkManager::SimulatorNetworkManager(const SimulatorNetworkManagerSettings& settings)
    : m_settings(settings) {

    m_context = std::make_shared<zmq::context_t>(1);

    createCameraAddressMap(settings.cameraPublisher);

    m_simulatorCommandServer.reset(new SimulatorCommandServer(m_context, m_settings, m_cameraAddresses));

    m_cameraPublisherGroup.reset(new CameraPublisherGroup(
        m_context, m_cameraAddresses, m_settings.network.zmqLingerTimeMs, m_settings.cameraPublisher.zmqHighWaterMarkValue));

    m_statsPublisher.reset(new SimulatorStatsPublisher(m_context, m_settings.statsPublisher.statsAddress,
        m_settings.network.zmqLingerTimeMs, m_settings.statsPublisher.zmqHighWaterMarkValue));
}

void SimulatorNetworkManager::update() { m_simulatorCommandServer->processCommandQueue(); }

void SimulatorNetworkManager::createCameraAddressMap(const SimulatorCameraPublisherSettings& settings) {

    m_cameraAddresses[hal::CameraId::FrontLeftStereo].image = std::string(settings.frontLeftStereoAddress);
    m_cameraAddresses[hal::CameraId::FrontLeftStereo].depth = std::string(settings.frontLeftStereoDepthAddress);
    m_cameraAddresses[hal::CameraId::FrontLeftStereo].pointcloud = std::string(settings.frontLeftStereoPointcloudAddress);

    m_cameraAddresses[hal::CameraId::FrontRightStereo].image = std::string(settings.frontRightStereoAddress);
    m_cameraAddresses[hal::CameraId::FrontRightStereo].depth = std::string(settings.frontRightStereoDepthAddress);
    m_cameraAddresses[hal::CameraId::FrontRightStereo].pointcloud = std::string(settings.frontRightStereoPointcloudAddress);

    m_cameraAddresses[hal::CameraId::RearLeftStereo].image = std::string(settings.rearLeftStereoAddress);
    m_cameraAddresses[hal::CameraId::RearLeftStereo].depth = std::string(settings.rearLeftStereoDepthAddress);
    m_cameraAddresses[hal::CameraId::RearLeftStereo].pointcloud = std::string(settings.rearLeftStereoPointcloudAddress);

    m_cameraAddresses[hal::CameraId::RearRightStereo].image = std::string(settings.rearRightStereoAddress);
    m_cameraAddresses[hal::CameraId::RearRightStereo].depth = std::string(settings.rearRightStereoDepthAddress);
    m_cameraAddresses[hal::CameraId::RearRightStereo].pointcloud = std::string(settings.rearRightStereoPointcloudAddress);

    m_cameraAddresses[hal::CameraId::FrontFisheye].image = std::string(settings.frontFisheyeAddress);
    m_cameraAddresses[hal::CameraId::FrontFisheye].depth = std::string(settings.frontFisheyeDepthAddress);
    m_cameraAddresses[hal::CameraId::FrontFisheye].pointcloud = std::string(settings.frontFisheyePointcloudAddress);

    m_cameraAddresses[hal::CameraId::RearFisheye].image = std::string(settings.rearFisheyeAddress);
    m_cameraAddresses[hal::CameraId::RearFisheye].depth = std::string(settings.rearFisheyeDepthAddress);
    m_cameraAddresses[hal::CameraId::RearFisheye].pointcloud = std::string(settings.rearFisheyePointcloudAddress);

    m_cameraAddresses[hal::CameraId::LeftFisheye].image = std::string(settings.leftFisheyeAddress);
    m_cameraAddresses[hal::CameraId::LeftFisheye].depth = std::string(settings.leftFisheyeDepthAddress);
    m_cameraAddresses[hal::CameraId::LeftFisheye].pointcloud = std::string(settings.leftFisheyePointcloudAddress);

    m_cameraAddresses[hal::CameraId::RightFisheye].image = std::string(settings.rightFisheyeAddress);
    m_cameraAddresses[hal::CameraId::RightFisheye].depth = std::string(settings.rightFisheyeDepthAddress);
    m_cameraAddresses[hal::CameraId::RightFisheye].pointcloud = std::string(settings.rightFisheyePointcloudAddress);
}
