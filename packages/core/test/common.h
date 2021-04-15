#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "glog/logging.h"
#include <boost/interprocess/managed_shared_memory.hpp>

namespace core {
namespace test {

    using boost::interprocess::create_only;
    using boost::interprocess::open_only;
    using boost::interprocess::managed_shared_memory;
    using boost::interprocess::shared_memory_object;

    static constexpr uint16_t kStartingPort = 16000;
    static constexpr char kDefaultSharedMemoryTag[] = "port_counter";
    static constexpr char kDefaultSharedMemoryDescriptor[] = "core_test_shared_memory";

    /*
     *A port-number provider, that can be used across threads/processes and is
     *  guaranteed to provide strictly increasing port counts
     */
    class UniquePortProvider {
    public:
        static void reset() { shared_memory_object::remove(kDefaultSharedMemoryDescriptor); }

        UniquePortProvider()
            : m_managed(nullptr) {
            try {
                createOnly();
            } catch (std::exception&) {
                openOnly();
            }
        }

        ~UniquePortProvider() {
            if (m_managed != nullptr) {
                delete m_managed;
                m_managed = nullptr;
            }
        }

        uint16_t next_port() {
            uint16_t m_port = 0;
            auto managed_memory = m_managed;
            auto increment = [&m_port, &managed_memory]() {
                CHECK_NOTNULL(managed_memory);
                auto result = managed_memory->find<uint16_t>(kDefaultSharedMemoryTag);
                CHECK_NOTNULL(result.first);
                CHECK(result.second == 1);

                int sockfd = socket(AF_INET, SOCK_STREAM, 0);
                CHECK(sockfd >= 0);
                int reuse_address = 1;
                setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&reuse_address), sizeof(reuse_address));
                struct sockaddr_in address;
                address.sin_family = AF_INET;
                address.sin_addr.s_addr = inet_addr("127.0.0.1");

                bool port_available = false;

                while (!port_available) {
                    m_port = *result.first;
                    address.sin_port = htons(m_port);
                    int success = bind(sockfd, reinterpret_cast<sockaddr*>(&address), sizeof(address));
                    (*result.first)++;
                    switch (success) {
                    case 0:
                        CHECK(close(sockfd) == 0);
                        port_available = true;
                        break;
                    case -1:
                        LOG(ERROR) << "Failed: " << m_port << ": " << strerror(errno);
                        break;
                    }
                }

            };
            m_managed->atomic_func(increment);
            CHECK(m_port != 0);
            return m_port;
        }

    private:
        UniquePortProvider(UniquePortProvider&) = delete;
        UniquePortProvider(UniquePortProvider&&) = delete;
        UniquePortProvider& operator=(UniquePortProvider&) = delete;
        UniquePortProvider& operator=(UniquePortProvider&&) = delete;

        void createOnly() {
            m_managed = new managed_shared_memory(create_only, kDefaultSharedMemoryDescriptor, 512);
            CHECK_NOTNULL(m_managed);
            auto result = m_managed->construct<uint16_t>(kDefaultSharedMemoryTag)(kStartingPort);
            CHECK_NOTNULL(result);
        }

        void openOnly() {
            m_managed = new managed_shared_memory(open_only, kDefaultSharedMemoryDescriptor);
            CHECK_NOTNULL(m_managed);
        }

        managed_shared_memory* m_managed;
    };

} // test
} // core
