#include "gtest/gtest.h"

#include "packages/core/test/common.h"
#include "packages/hal/proto/camera_id.pb.h"
#include "packages/net/include/zmq_req_client.h"
#include "packages/unity_plugins/simulated_cameras/include/simulator_network_interop.h"
#include "packages/unity_plugins/simulated_cameras/include/simulator_network_manager.h"
#include "packages/unity_plugins/simulated_cameras/include/simulator_publisher_topics.h"

void cameraConfigCallback(unity_plugins::CameraConfigurationCommand) {}

void resetCallback(unity_plugins::SimulatorResetCommand) {}

void statsCallback(unity_plugins::StatsCommand) {}

unity_plugins::SimulatorNetworkManagerSettings createTestSettings() {
    core::test::UniquePortProvider provider;

    unity_plugins::SimulatorNetworkManagerSettings settings;

    settings.network.zmqLingerTimeMs = 10;
    settings.network.zmqSendRecvTimeoutMs = 10;

    strcpy(settings.commandServer.serverAddress, "tcp://127.0.0.1:5401\0");
    settings.commandServer.cameraConfigCallback = cameraConfigCallback;
    settings.commandServer.resetCallback = resetCallback;
    settings.commandServer.statsCallback = statsCallback;

    strcpy(settings.cameraPublisher.frontLeftStereoAddress, "tcp://127.0.0.1:5501\0");
    strcpy(settings.cameraPublisher.frontLeftStereoDepthAddress, "tcp://127.0.0.1:5502\0");
    strcpy(settings.cameraPublisher.frontLeftStereoPointcloudAddress, "tcp://127.0.0.1:5503\0");

    strcpy(settings.cameraPublisher.frontRightStereoAddress, "tcp://127.0.0.1:5504\0");
    strcpy(settings.cameraPublisher.frontRightStereoDepthAddress, "tcp://127.0.0.1:5505\0");
    strcpy(settings.cameraPublisher.frontRightStereoPointcloudAddress, "tcp://127.0.0.1:5506\0");

    strcpy(settings.cameraPublisher.rearLeftStereoAddress, "tcp://127.0.0.1:5507\0");
    strcpy(settings.cameraPublisher.rearLeftStereoDepthAddress, "tcp://127.0.0.1:5508\0");
    strcpy(settings.cameraPublisher.rearLeftStereoPointcloudAddress, "tcp://127.0.0.1:5509\0");

    strcpy(settings.cameraPublisher.rearRightStereoAddress, "tcp://127.0.0.1:5510\0");
    strcpy(settings.cameraPublisher.rearRightStereoDepthAddress, "tcp://127.0.0.1:5511\0");
    strcpy(settings.cameraPublisher.rearRightStereoPointcloudAddress, "tcp://127.0.0.1:5512\0");

    strcpy(settings.cameraPublisher.frontFisheyeAddress, "tcp://127.0.0.1:5513\0");
    strcpy(settings.cameraPublisher.frontFisheyeDepthAddress, "tcp://127.0.0.1:5514\0");
    strcpy(settings.cameraPublisher.frontFisheyePointcloudAddress, "tcp://127.0.0.1:5515\0");

    strcpy(settings.cameraPublisher.rearFisheyeAddress, "tcp://127.0.0.1:5516\0");
    strcpy(settings.cameraPublisher.rearFisheyeDepthAddress, "tcp://127.0.0.1:5517\0");
    strcpy(settings.cameraPublisher.rearFisheyePointcloudAddress, "tcp://127.0.0.1:5518\0");

    strcpy(settings.cameraPublisher.leftFisheyeAddress, "tcp://127.0.0.1:5519\0");
    strcpy(settings.cameraPublisher.leftFisheyeDepthAddress, "tcp://127.0.0.1:5520\0");
    strcpy(settings.cameraPublisher.leftFisheyePointcloudAddress, "tcp://127.0.0.1:5521\0");

    strcpy(settings.cameraPublisher.rightFisheyeAddress, "tcp://127.0.0.1:5522\0");
    strcpy(settings.cameraPublisher.rightFisheyeDepthAddress, "tcp://127.0.0.1:5523\0");
    strcpy(settings.cameraPublisher.rightFisheyePointcloudAddress, "tcp://127.0.0.1:5524\0");

    settings.cameraPublisher.zmqHighWaterMarkValue = 1;

    strcpy(settings.statsPublisher.statsAddress, "tcp://127.0.0.1:6001\0");

    return settings;
}

TEST(SimulatedCameras, ConstructAndDestruct) {
    auto settings = createTestSettings();

    unity_plugins::SimulatorNetworkManager simulatedCameras(settings);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(SimulatedCameras, SendCameraConfigCommand) {

    auto settings = createTestSettings();
    unity_plugins::SimulatorNetworkManager simulatedCameras(settings);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    zmq::context_t context(1);
    std::string addr = std::string(settings.commandServer.serverAddress);
    net::ZMQProtobufReqClient<hal::SimulatorCommandEnvelope, hal::SimulatorCommandResponse> client(context, addr, 1000, 1000);

    hal::SimulatorCommandEnvelope commandMsg;
    auto command = new hal::SimulatorCameraCommand();
    command->set_cameraid(hal::CameraId::FrontFisheye);
    command->set_enable(true);
    command->set_enabledepth(false);
    command->set_enablexyz(true);

    commandMsg.set_allocated_cameracommand(command);

    DLOG(INFO) << "Simulator Command Client : Sending message.";

    EXPECT_TRUE(client.send(commandMsg));

    //  Get the reply.
    hal::SimulatorCommandResponse response;
    EXPECT_TRUE(client.recv(response));

    EXPECT_EQ(response.status(), hal::StatusCode::StatusSuccess);

    EXPECT_TRUE(response.has_cameraresponse());
    EXPECT_FALSE(response.has_resetresponse());
    EXPECT_FALSE(response.has_statsresponse());

    const std::string expectedImagePublisherAddress = std::string(settings.cameraPublisher.frontFisheyeAddress);
    const std::string expectedDepthPublisherAddress = std::string(settings.cameraPublisher.frontFisheyeDepthAddress);
    const std::string expectedPointcloudPublisherAddress = std::string(settings.cameraPublisher.frontFisheyePointcloudAddress);

    EXPECT_EQ(response.cameraresponse().image().address().compare(expectedImagePublisherAddress), 0);
    EXPECT_EQ(response.cameraresponse().depth().address().compare(expectedDepthPublisherAddress), 0);
    EXPECT_EQ(response.cameraresponse().xyz().address().compare(expectedPointcloudPublisherAddress), 0);

    EXPECT_EQ(response.cameraresponse().image().topic().compare(unity_plugins::CAMERA_OUTPUT_TOPIC), 0);
    EXPECT_EQ(response.cameraresponse().depth().topic().compare(unity_plugins::CAMERA_OUTPUT_TOPIC), 0);
    EXPECT_EQ(response.cameraresponse().xyz().topic().compare(unity_plugins::CAMERA_OUTPUT_TOPIC), 0);

    DLOG(INFO) << "Simulator Command Client : Received a message, size : " << response.ByteSize();
}

TEST(SimulatedCameras, SendResetCommand) {

    auto settings = createTestSettings();
    unity_plugins::SimulatorNetworkManager simulatedCameras(settings);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    zmq::context_t context(1);
    std::string addr = std::string(settings.commandServer.serverAddress);
    net::ZMQProtobufReqClient<hal::SimulatorCommandEnvelope, hal::SimulatorCommandResponse> client(context, addr, 1000, 1000);

    hal::SimulatorCommandEnvelope commandMsg;
    auto command = new hal::SimulatorResetCommand();
    command->set_resetrobot(true);
    command->set_resetworld(false);
    command->set_resetstreams(true);

    commandMsg.set_allocated_resetcommand(command);

    DLOG(INFO) << "Simulator Command Client : Sending message.";

    EXPECT_TRUE(client.send(commandMsg));

    //  Get the reply.
    hal::SimulatorCommandResponse response;
    EXPECT_TRUE(client.recv(response));

    EXPECT_EQ(response.status(), hal::StatusCode::StatusSuccess);

    EXPECT_FALSE(response.has_cameraresponse());
    EXPECT_TRUE(response.has_resetresponse());
    EXPECT_FALSE(response.has_statsresponse());

    DLOG(INFO) << "Simulator Command Client : Received a message, size : " << response.ByteSize();
}

TEST(SimulatedCameras, SendStatsCommand) {

    auto settings = createTestSettings();
    unity_plugins::SimulatorNetworkManager simulatedCameras(settings);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    zmq::context_t context(1);
    std::string addr = std::string(settings.commandServer.serverAddress);
    net::ZMQProtobufReqClient<hal::SimulatorCommandEnvelope, hal::SimulatorCommandResponse> client(context, addr, 1000, 1000);

    hal::SimulatorCommandEnvelope commandMsg;
    auto command = new hal::SimulatorStatsCommand();
    command->set_enable(true);
    commandMsg.set_allocated_statscommand(command);

    DLOG(INFO) << "Simulator Command Client : Sending message.";

    EXPECT_TRUE(client.send(commandMsg));

    //  Get the reply.
    hal::SimulatorCommandResponse response;
    EXPECT_TRUE(client.recv(response));

    EXPECT_EQ(response.status(), hal::StatusCode::StatusSuccess);

    EXPECT_FALSE(response.has_resetresponse());
    EXPECT_FALSE(response.has_cameraresponse());
    EXPECT_TRUE(response.has_statsresponse());

    DLOG(INFO) << "Simulator Command Client : Received a message, size : " << response.ByteSize();
}