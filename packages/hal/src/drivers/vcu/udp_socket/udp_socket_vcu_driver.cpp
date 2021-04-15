#include "packages/hal/include/drivers/vcu/udp_socket/udp_socket_vcu_driver.h"

#include "glog/logging.h"

namespace hal {

namespace vcu {

    constexpr int BUFFER_SIZE = 2048;

    UdpSocketVcuDriver::UdpSocketVcuDriver(
        const std::string& vcuAddress, const unsigned short& commandPort, const unsigned short& telemetryPort) {

        LOG(INFO) << "Creating UDP Socket VCU device with VCU Address: " << vcuAddress << " Command Port: " << commandPort
                  << " Telemetry Port: " << telemetryPort;
        m_commandSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (m_commandSocket < 0) {
            LOG(ERROR) << "UdpSocketVcu: Cannot create command socket";
            throw std::runtime_error("UdpSocketVcu: Cannot create command socket");
        }

        /// Create a sockaddr_in data structure with the VCU address in the proper format
        memset((char*)&m_commandAddress, 0, sizeof(m_commandAddress));
        m_commandAddress.sin_family = AF_INET;
        m_commandAddress.sin_port = htons(commandPort);
        if (inet_aton(vcuAddress.c_str(), &m_commandAddress.sin_addr) == 0) {
            LOG(ERROR) << "UdpSocketVcu: inet_aton() failed";
            throw std::runtime_error("UdpSocketVcu: inet_aton() failed");
        }

        /// Set the timeout for receiving a response to a command sent to the VCU
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        if (setsockopt(m_commandSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            LOG(ERROR) << "UdpSocketVcu: Set receive timeout failed";
            throw std::runtime_error("UdpSocketVcu: Set receive timeout failed");
        }

        /// Connect to the VCU on the command channel
        if (connect(m_commandSocket, (struct sockaddr*)&m_commandAddress, sizeof(m_commandAddress)) < 0) {
            LOG(ERROR) << "UdpSocketVcu: Failed to connect to VCU on command channel";
            throw std::runtime_error("UdpSocketVcu: Failed to connect to VCU on command channel");
        }

        /// Create a socket for receiving telemetry data
        m_telemetrySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (m_telemetrySocket < 0) {
            LOG(ERROR) << "UdpSocketVcu: Cannot create telemetry socket";
            throw std::runtime_error("UdpSocketVcu: Cannot create telemetry socket");
        }

        /// Create a sockaddr_in datastructure to receive telemetry data from the VCU
        memset((char*)&m_telemetryAddress, 0, sizeof(m_telemetryAddress));
        m_telemetryAddress.sin_family = AF_INET;
        m_telemetryAddress.sin_port = htons(telemetryPort);
        m_telemetryAddress.sin_addr.s_addr = htonl(INADDR_ANY);

        /// Bind the telemetry socket to receive telemetry messages sent to the port
        if (bind(m_telemetrySocket, (struct sockaddr*)&m_telemetryAddress, sizeof(m_telemetryAddress)) < 0) {
            LOG(ERROR) << "UdpSocketVcu: Failed to connect to VCU on telemetry channel";
            throw std::runtime_error("UdpSocketVcu: Failed to connect to VCU on telemetry channel");
        }
        LOG(INFO) << "Created UDP Socket VCU device with VCU Address: " << vcuAddress << " Command Port: " << commandPort
                  << " Telemetry Port: " << telemetryPort;
    }

    bool UdpSocketVcuDriver::poll(uint32_t timeoutInMicroseconds) {

        fd_set fdSet;
        struct timeval timeout;

        /// Set timeout duration
        timeout.tv_sec = timeoutInMicroseconds / 1000000;
        timeout.tv_usec = timeoutInMicroseconds % 1000000;

        /// Create a descriptor containing the telemetry socket
        FD_ZERO(&fdSet);
        FD_SET(m_telemetrySocket, &fdSet);

        /// Select returns the number of sockets that have data waiting to be read
        /// Since we have only one socket, non-zero retval means there is telemetry data to read
        int retval = select(m_telemetrySocket + 1, &fdSet, NULL, NULL, &timeout);
        if (retval < 0) {
            LOG(ERROR) << "UdpSocketVcu: Select command failed";
            throw std::runtime_error("UdpSocketVcu: Select command failed");
        } else if (retval == 0) {
            LOG_EVERY_N(INFO, 1000) << "UdpSocketVcu: No data on socket at port: " << m_telemetryAddress.sin_port;
            return false;
        } else {
            return true;
        }
    }

    bool UdpSocketVcuDriver::capture(VCUTelemetryEnvelope& telemetryEnvelope) {

        /// Receive telemetry data from the VCU
        std::vector<unsigned char> buffer(BUFFER_SIZE);
        ssize_t bytesRead = recv(m_telemetrySocket, buffer.data(), BUFFER_SIZE, 0);
        if (bytesRead < 0) {
            if (errno == EAGAIN) {
                LOG(ERROR) << "UdpSocketVcu: Recieve telemetry failed";
                return false;
            }
            LOG(ERROR) << "UdpSocketVcu: Receive telemetry failed";
            throw std::runtime_error("UdpSocketVcu: Receive telemetry failed");
        }

        telemetryEnvelope.ParseFromArray(buffer.data(), bytesRead);
        LOG_EVERY_N(INFO, 1000) << "UdpSocketVcu: Captured telemetry sample";
        return true;
    }

    bool UdpSocketVcuDriver::send(const VCUCommandEnvelope& commandEnvelope, VCUCommandResponse& commandResponse) {

        std::string command = commandEnvelope.SerializeAsString();

        /// Send serialized protobuf command
        if (sendto(m_commandSocket, command.c_str(), command.size(), 0, (struct sockaddr*)&m_commandAddress, sizeof(m_commandAddress))
            == -1) {
            LOG(ERROR) << "UdpSocketVcu: Send command failed";
            throw std::runtime_error("UdpSocketVcu: Send command failed");
        }

        /// Receive command response from VCU
        std::vector<unsigned char> buffer(BUFFER_SIZE);
        ssize_t bytesRead = recv(m_commandSocket, buffer.data(), BUFFER_SIZE, 0);
        if (bytesRead < 0) {
            if (errno == EAGAIN) {
                LOG(ERROR) << "UdpSocketVcu: Recieve command timeout";
                return false;
            }
            LOG(ERROR) << "UdpSocketVcu: Receive command failed";
            throw std::runtime_error("UdpSocketVcu: Receive command failed");
        }

        commandResponse.ParseFromArray(buffer.data(), bytesRead);
        LOG(INFO) << "UdpSocketVcu: Sent VCU command and received a response";

        return true;
    }

    UdpSocketVcuDriver::~UdpSocketVcuDriver() {
        /// Close sockets
        close(m_commandSocket);
        close(m_telemetrySocket);
    }
} // vcu
} // hal
