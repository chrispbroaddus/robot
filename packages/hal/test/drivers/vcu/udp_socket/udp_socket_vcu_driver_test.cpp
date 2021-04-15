#include "packages/hal/include/drivers/vcu/udp_socket/udp_socket_vcu_driver.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

#include <thread>

using namespace hal;
using namespace hal::vcu;

namespace {

class HalUdpSocketVcuDriverFixtureTest : public ::testing::Test {
protected:
    void SetUp() {
        LOG(INFO) << "TestFixture: Setting up test fixture";

        m_commandSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (m_commandSocket < 0) {
            LOG(ERROR) << "TestFixture: Cannot create test command socket";
            throw std::runtime_error("TestFixture: Cannot create test command socket");
        }

        memset((char*)&m_serverAddress, 0, sizeof(m_serverAddress));
        m_serverAddress.sin_family = AF_INET;
        m_serverAddress.sin_port = htons(10000);
        m_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        if (setsockopt(m_commandSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            LOG(ERROR) << "TestFixture: Set receive timeout failed";
            throw std::runtime_error("TestFixture: Set receive timeout failed");
        }

        if (bind(m_commandSocket, (struct sockaddr*)&m_serverAddress, sizeof(m_serverAddress)) < 0) {
            LOG(ERROR) << "TestFixture: Cannot bind to command endpoint";
            throw std::runtime_error("TestFixture: Cannot bind to command endpoint");
        }
        LOG(INFO) << "TestFixture: Created command server";

        m_telemetrySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (m_telemetrySocket < 0) {
            LOG(ERROR) << "TestFixture: Cannot create telemetry socket";
            throw std::runtime_error("TestFixture: Cannot create telemetry socket");
        }

        memset((char*)&m_telemetryAddress, 0, sizeof(m_telemetryAddress));
        m_telemetryAddress.sin_family = AF_INET;
        m_telemetryAddress.sin_port = htons(10002);
        m_telemetryAddress.sin_addr.s_addr = htonl(INADDR_ANY);

        if (connect(m_telemetrySocket, (struct sockaddr*)&m_telemetryAddress, sizeof(m_telemetryAddress)) < 0) {
            LOG(ERROR) << "TestFixture: Failed to connect to host on telemetry channel";
            throw std::runtime_error("TestFixture: Failed to connect to host on telemetry channel");
        }
        LOG(INFO) << "TestFixture: Created telemetry socket";
    }

    void TearDown() {
        close(m_commandSocket);
        close(m_telemetrySocket);
    }

public:
    bool recvCommand() {
        LOG(INFO) << "TestFixture: In receive command";
        /// Receive protobuf command
        std::vector<unsigned char> buffer(m_bufferSize);
        socklen_t sockAddressLen = sizeof(m_commandAddress);
        ssize_t bytesRead = recvfrom(m_commandSocket, buffer.data(), m_bufferSize, 0, (struct sockaddr*)&m_commandAddress, &sockAddressLen);
        if (bytesRead < 0) {
            if (errno == EAGAIN) {
                LOG(ERROR) << "TestFixture: Recieve command timeout";
                return false;
            }
            LOG(ERROR) << "TestFixture: Receive command failed";
            throw std::runtime_error("TestFixture: Receive command failed");
        }

        /// Parse received command
        VCUCommandEnvelope commandEnvelope;
        commandEnvelope.ParseFromArray(buffer.data(), bytesRead);

        /// Generate response if the expected command was received
        VCUCommandResponse commandResponse;
        if (commandEnvelope.sequencenumber() == 1234) {
            commandResponse.set_disposition(CommandAccepted);
        } else {
            commandResponse.set_disposition(CommandRejected);
        }

        LOG(INFO) << "TestFixture: Sending response: " << commandResponse.DebugString();
        /// Send serialized protobuf response
        std::string response = commandResponse.SerializeAsString();
        if (sendto(m_commandSocket, response.c_str(), response.size(), 0, (const struct sockaddr*)&m_commandAddress, sockAddressLen)
            == -1) {
            LOG(ERROR) << "TestFixture: Send command failed";
            throw std::runtime_error("TestFixture: Send command failed");
        }
        return true;
    }

    bool sendTelemetry() {

        VCUTelemetryEnvelope telemetryEnvelope;
        core::SystemTimestamp* timestamp = new core::SystemTimestamp();

        timestamp->set_nanos(1234);
        telemetryEnvelope.set_allocated_sendtimestamp(timestamp);

        /// Send serialized telemetry protobuf
        std::string telemetry = telemetryEnvelope.SerializeAsString();
        if (sendto(m_telemetrySocket, telemetry.c_str(), telemetry.size(), 0, (struct sockaddr*)&m_telemetryAddress,
                sizeof(m_telemetryAddress))
            == -1) {
            LOG(ERROR) << "TestFixture: Send telemetry failed";
            throw std::runtime_error("TestFixture: Send telemetry failed");
        }
        return true;
    }

private:
    int m_bufferSize = 2048;
    int m_commandSocket;
    int m_telemetrySocket;
    struct sockaddr_in m_commandAddress;
    struct sockaddr_in m_telemetryAddress;
    struct sockaddr_in m_serverAddress;
};

TEST(HalUdpSocketVcuDriverTest, canInstantiate) {
    const std::string vcuAddress = "0.0.0.0";
    const unsigned short commandPort = 10000;
    const unsigned short telemetryPort = 10002;

    EXPECT_NO_THROW(UdpSocketVcuDriver udpSocketVcuDriver(vcuAddress, commandPort, telemetryPort));
}

TEST_F(HalUdpSocketVcuDriverFixtureTest, canReceiveTelemetry) {
    const std::string vcuAddress = "0.0.0.0";
    const unsigned short commandPort = 10000;
    const unsigned short telemetryPort = 10002;

    UdpSocketVcuDriver udpSocketVcuDriver(vcuAddress, commandPort, telemetryPort);

    if (!sendTelemetry()) {
        LOG(ERROR) << "TextFixture: Send telemetry failed";
        throw std::runtime_error("TextFixture: Send telemetry failed");
    }
    if (!udpSocketVcuDriver.poll(1000000)) {
        throw std::runtime_error("UdpSocketVcu: Failed to receive telemetry");
    }
    VCUTelemetryEnvelope telemetryEnvelope;
    udpSocketVcuDriver.capture(telemetryEnvelope);
    EXPECT_EQ(telemetryEnvelope.sendtimestamp().nanos(), 1234);
}

TEST_F(HalUdpSocketVcuDriverFixtureTest, canSendCommands) {
    const std::string vcuAddress = "0.0.0.0";
    const unsigned short commandPort = 10000;
    const unsigned short telemetryPort = 10002;

    UdpSocketVcuDriver udpSocketVcuDriver(vcuAddress, commandPort, telemetryPort);

    std::thread receiveThread(&HalUdpSocketVcuDriverFixtureTest::recvCommand, this);

    VCUCommandEnvelope commandEnvelope;
    VCUCommandResponse commandResponse;
    commandEnvelope.set_sequencenumber(1234);

    udpSocketVcuDriver.send(commandEnvelope, commandResponse);
    EXPECT_EQ(commandResponse.disposition(), CommandAccepted);
    receiveThread.join();
}

TEST_F(HalUdpSocketVcuDriverFixtureTest, canSendAndReceive) {
    const std::string vcuAddress = "0.0.0.0";
    const unsigned short commandPort = 10000;
    const unsigned short telemetryPort = 10002;

    UdpSocketVcuDriver udpSocketVcuDriver(vcuAddress, commandPort, telemetryPort);

    std::thread receiveThread(&HalUdpSocketVcuDriverFixtureTest::recvCommand, this);

    if (!sendTelemetry()) {
        LOG(ERROR) << "TextFixture: Send telemetry failed";
        throw std::runtime_error("TextFixture: Send telemetry failed");
    }
    if (!udpSocketVcuDriver.poll(1000000)) {
        throw std::runtime_error("UdpSocketVcu: Failed to receive telemetry");
    }
    VCUTelemetryEnvelope telemetryEnvelope;
    udpSocketVcuDriver.capture(telemetryEnvelope);
    EXPECT_EQ(telemetryEnvelope.sendtimestamp().nanos(), 1234);

    VCUCommandEnvelope commandEnvelope;
    VCUCommandResponse commandResponse;
    commandEnvelope.set_sequencenumber(1234);

    udpSocketVcuDriver.send(commandEnvelope, commandResponse);
    EXPECT_EQ(commandResponse.disposition(), CommandAccepted);
    receiveThread.join();
}
}
