#include "packages/core/test/common.h"
#include "gtest/gtest.h"
#include <string>
#include <thread>

#include <zmq.h>

namespace core {
namespace test {

    class PortRequestor {
    public:
        PortRequestor()
            : m_request_thread()
            , m_ports() {}

        void request(UniquePortProvider* provider, int num_requests) {
            CHECK_NOTNULL(provider);
            CHECK(num_requests > 0);
            m_request_thread = std::thread(&PortRequestor::run, this, provider, num_requests);
        }

        void wait() { m_request_thread.join(); }

        ~PortRequestor() {
            if (m_request_thread.joinable()) {
                m_request_thread.join();
            }
        }

        const std::vector<uint16_t>& ports() const { return m_ports; }

    private:
        void run(UniquePortProvider* provider, int num_requests) {
            for (int i = 0; i < num_requests; ++i) {
                m_ports.emplace_back(provider->next_port());
            }
        }
        std::thread m_request_thread;
        std::vector<uint16_t> m_ports;
    };

} // test
} // core

// Check threaded access to the port provider, using two simultaneous requestors
TEST(portProvider, threaded) {
    using core::test::UniquePortProvider;
    UniquePortProvider::reset();
    UniquePortProvider provider;

    core::test::PortRequestor requestor_1;
    core::test::PortRequestor requestor_2;
    requestor_1.request(&provider, 100);
    requestor_2.request(&provider, 300);
    requestor_1.wait();
    requestor_2.wait();

    const auto& requestor_1_ports = requestor_1.ports();
    const auto& requestor_2_ports = requestor_2.ports();

    std::vector<uint16_t> common_ports;
    std::set_intersection(requestor_1_ports.begin(), requestor_1_ports.end(), requestor_2_ports.begin(), requestor_2_ports.end(),
        std::back_inserter(common_ports));
    ASSERT_TRUE(common_ports.empty());
}

TEST(portProvider, clash) {
    using core::test::UniquePortProvider;
    UniquePortProvider::reset();
    UniquePortProvider provider;
    auto port = provider.next_port();

    void* context = zmq_ctx_new();

    constexpr int kNumReservedPorts = 10;
    std::vector<void*> reserved(kNumReservedPorts, 0);
    int counter = 0;
    for (auto& reserved_port : reserved) {
        reserved_port = zmq_socket(context, ZMQ_REP);
        std::string address = "tcp://*:" + std::to_string(port + (counter++));
        int m_serverStatus = zmq_bind(reserved_port, address.c_str());
        ASSERT_EQ(m_serverStatus, 0);
    }
    for (auto& reserved_port : reserved) {
        ASSERT_EQ(zmq_close(reserved_port), 0);
    }
}
