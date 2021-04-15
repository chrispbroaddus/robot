#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <netinet/in.h>
#include <sys/socket.h>

#include "packages/hal/proto/vcu_status_request.pb.h"
#include "packages/hal/proto/vcu_status_response.pb.h"

#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_string(address, "", "VCU address to connect to");
DEFINE_int32(commandPort, 0, "VCU command port to connect to");

std::atomic_bool s_active(false);

namespace {

class UdpSocketVcuQuery {
public:
    UdpSocketVcuQuery(const std::string& address, const int commandPort) {
        m_requestSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        CHECK(m_requestSocket > 0) << "Cannot create request socket";
        memset((char*)&m_requestAddress, 0, sizeof(m_requestAddress));
        m_requestAddress.sin_family = AF_INET;
        m_requestAddress.sin_port = htons(commandPort);
        CHECK(inet_aton(address.c_str(), &m_requestAddress.sin_addr) != 0) << "Udp socket inet_aton() failed";

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        // Set socket options
        CHECK(setsockopt(m_requestSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) >= 0) << "Failed to set timeout value";
        // Connect
        CHECK(connect(m_requestSocket, (struct sockaddr*)&m_requestAddress, sizeof(m_requestAddress)) >= 0) << "Failed to connect";
    }

    ~UdpSocketVcuQuery() { close(m_requestSocket); }

    bool sendAndReceive(const hal::VCUStatusRequest& request, hal::VCUStatusResponse& response) {
        std::string command = request.SerializeAsString();

        /// Send serialized protobuf command
        if (sendto(m_requestSocket, command.c_str(), command.size(), 0, (struct sockaddr*)&m_requestAddress, sizeof(m_requestAddress))
            == -1) {
            LOG(ERROR) << "Send command failed";
            throw std::runtime_error("Send command failed");
        }

        /// Receive command response from VCU
        constexpr int BUFFER_SIZE = 2048;
        std::vector<unsigned char> buffer(BUFFER_SIZE);
        ssize_t bytesRead = recv(m_requestSocket, buffer.data(), BUFFER_SIZE, 0);
        if (bytesRead < 0) {
            if (errno == EAGAIN) {
                LOG(ERROR) << "Command timeout";
                return false;
            }
            LOG(ERROR) << "Receive command failed";
            throw std::runtime_error("Receive command failed");
        }
        response.ParseFromArray(buffer.data(), bytesRead);
        return true;
    }

private:
    int m_requestSocket;
    struct sockaddr_in m_requestAddress;
};
}

int main(int argc, char** argv) {
    s_active = true;

    gflags::SetUsageMessage("VCU status requester");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    UdpSocketVcuQuery driver(FLAGS_address, FLAGS_commandPort);
    auto timer = std::chrono::high_resolution_clock::now;

    auto lastCaptureTime = timer();

    int requestCounter = 0;

    while (s_active) {
        if (std::chrono::duration_cast<std::chrono::seconds>(timer() - lastCaptureTime) > std::chrono::seconds(1)) {
            hal::VCUStatusRequest request;
            request.set_sequencenumber(++requestCounter);
            request.set_statusid(hal::Version);
            hal::VCUStatusResponse response;
            if (driver.sendAndReceive(request, response)) {
                LOG(INFO) << response.DebugString();
            }
            lastCaptureTime = timer();
        }
    }
    return EXIT_SUCCESS;
}
