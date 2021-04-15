#include "packages/hal/include/drivers/vcu/unity_simulated/unity_simulated_vcu.h"
#include "packages/net/include/zmq_rep_server.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "gtest/gtest.h"

#include <iostream>
#include <thread>

using namespace hal;
using namespace hal::vcu;

const double kSecondsToNanoseconds = 1e+9;

namespace vcuTest {

void serverThread() {
    zmq::context_t context = zmq::context_t(1);
    uint32_t lingerPeriodInMilliseconds = 1000;
    uint32_t sendRecvTimeouInMilliseconds = 500;
    net::ZMQProtobufRepServer<VCUCommandEnvelope, VCUCommandResponse> trajectoryRepServer(
        context, "tcp://*:5561", lingerPeriodInMilliseconds, sendRecvTimeouInMilliseconds);
    VCUCommandEnvelope commandEnvelope;
    VCUCommandResponse commandResponse;

    trajectoryRepServer.recv(commandEnvelope);

    if (commandEnvelope.sequencenumber() == 20) {
        commandResponse.set_disposition(CommandAccepted);
    } else {
        commandResponse.set_disposition(CommandRejected);
    }
    trajectoryRepServer.send(commandResponse);
}
}

TEST(HalUnitySimulatedVcuTest, canCreateVcu) {
    uint32_t lingerPeriodInMilliseconds = 1000;
    uint32_t sendRecvTimeouInMilliseconds = 500;
    uint32_t highWaterMark = 100;
    details::property_map_t deviceConfig;
    deviceConfig["repServerAddress"] = "tcp://localhost:5561";
    deviceConfig["lingerPeriodInMilliseconds"] = std::to_string(lingerPeriodInMilliseconds);
    deviceConfig["sendRecvTimeoutInMilliseconds"] = std::to_string(sendRecvTimeouInMilliseconds);
    deviceConfig["subscriberAddress"] = "tcp://localhost:5562";
    deviceConfig["subscriberTopic"] = "telemetry";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(highWaterMark);

    EXPECT_NO_THROW(UnitySimulatedVcu vcu(deviceConfig));
}

TEST(HalUnitySimulatedVcuTest, canReceiveTelemetryData) {
    zmq::context_t context = zmq::context_t(1);
    uint32_t lingerPeriodInMilliseconds = 1000;
    uint32_t sendRecvTimeouInMilliseconds = 500;
    uint32_t highWaterMark = 100;
    details::property_map_t deviceConfig;
    deviceConfig["repServerAddress"] = "tcp://localhost:5561";
    deviceConfig["lingerPeriodInMilliseconds"] = std::to_string(lingerPeriodInMilliseconds);
    deviceConfig["sendRecvTimeoutInMilliseconds"] = std::to_string(sendRecvTimeouInMilliseconds);
    deviceConfig["subscriberAddress"] = "tcp://localhost:5562";
    deviceConfig["subscriberTopic"] = "telemetry";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(highWaterMark);

    UnitySimulatedVcu vcu(deviceConfig);

    net::ZMQProtobufPublisher<VCUTelemetryEnvelope> telemetryPublisher(context, "tcp://*:5562", highWaterMark, lingerPeriodInMilliseconds);
    VCUTelemetryEnvelope telemetryEnvelope;
    VCUTelemetryEnvelope receivedTelemetryEnvelope;

    // Pub/Sub are not synchronized, so here we need to wait a bit until the Sub has had time to start.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_FALSE(vcu.poll(0));

    telemetryEnvelope.default_instance();
    telemetryEnvelope.mutable_sendtimestamp()->set_nanos(10 * kSecondsToNanoseconds);

    telemetryPublisher.send(telemetryEnvelope, "telemetry");

    // Pub/Sub are not synchronized, so here we need to wait a bit until the Sub has had time to start.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(vcu.poll(0));

    EXPECT_TRUE(vcu.capture(receivedTelemetryEnvelope));

    EXPECT_EQ(10 * kSecondsToNanoseconds, receivedTelemetryEnvelope.sendtimestamp().nanos());
}

TEST(HalUnitySimulatedVcuTest, canSendTrajectoryData) {
    std::thread testThread(vcuTest::serverThread);

    uint32_t lingerPeriodInMilliseconds = 1000;
    uint32_t sendRecvTimeouInMilliseconds = 500;
    uint32_t highWaterMark = 100;
    details::property_map_t deviceConfig;
    deviceConfig["repServerAddress"] = "tcp://localhost:5561";
    deviceConfig["lingerPeriodInMilliseconds"] = std::to_string(lingerPeriodInMilliseconds);
    deviceConfig["sendRecvTimeoutInMilliseconds"] = std::to_string(sendRecvTimeouInMilliseconds);
    deviceConfig["subscriberAddress"] = "tcp://localhost:5562";
    deviceConfig["subscriberTopic"] = "telemetry";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(highWaterMark);

    UnitySimulatedVcu vcu(deviceConfig);
    VCUCommandEnvelope commandEnvelope;
    VCUCommandResponse commandResponse;

    commandEnvelope.default_instance();
    commandEnvelope.set_sequencenumber(20);

    vcu.send(commandEnvelope, commandResponse);

    EXPECT_EQ(commandResponse.disposition(), CommandAccepted);

    testThread.join();
}

TEST(HalUnitySimulatedVcuTest, canSendTrajectoryRecvTelemetryData) {
    std::thread testThread(vcuTest::serverThread);

    zmq::context_t context = zmq::context_t(1);
    uint32_t lingerPeriodInMilliseconds = 1000;
    uint32_t sendRecvTimeouInMilliseconds = 500;
    uint32_t highWaterMark = 100;
    details::property_map_t deviceConfig;
    deviceConfig["repServerAddress"] = "tcp://localhost:5561";
    deviceConfig["lingerPeriodInMilliseconds"] = std::to_string(lingerPeriodInMilliseconds);
    deviceConfig["sendRecvTimeoutInMilliseconds"] = std::to_string(sendRecvTimeouInMilliseconds);
    deviceConfig["subscriberAddress"] = "tcp://localhost:5562";
    deviceConfig["subscriberTopic"] = "telemetry";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(highWaterMark);

    UnitySimulatedVcu vcu(deviceConfig);
    net::ZMQProtobufPublisher<VCUTelemetryEnvelope> telemetryPublisher(context, "tcp://*:5562", highWaterMark, lingerPeriodInMilliseconds);
    VCUCommandEnvelope commandEnvelope;
    VCUCommandResponse commandResponse;
    VCUTelemetryEnvelope telemetryEnvelope;
    VCUTelemetryEnvelope receivedTelemetryEnvelope;

    // Pub/Sub are not synchronized, so here we need to wait a bit until the Sub has had time to start.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_FALSE(vcu.poll(0));

    commandEnvelope.default_instance();
    commandEnvelope.set_sequencenumber(20);
    telemetryEnvelope.default_instance();
    telemetryEnvelope.mutable_sendtimestamp()->set_nanos(10 * kSecondsToNanoseconds);

    telemetryPublisher.send(telemetryEnvelope, "telemetry");

    vcu.send(commandEnvelope, commandResponse);

    // Pub/Sub are not synchronized, so here we need to wait a bit until the Sub has had time to start.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(vcu.poll(0));

    EXPECT_TRUE(vcu.capture(receivedTelemetryEnvelope));

    EXPECT_EQ(10 * kSecondsToNanoseconds, receivedTelemetryEnvelope.sendtimestamp().nanos());

    EXPECT_EQ(commandResponse.disposition(), CommandAccepted);

    testThread.join();
}
