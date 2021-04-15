#include "packages/hal/include/drivers/network_health//interface_statistics.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifdef __linux__
#include <linux/if.h>
#include <linux/if_link.h>
#else
#include <net/if.h>
#endif

#include <unistd.h>

#include <cerrno>
#include <stdexcept>
#include <string>

#include "glog/logging.h"

namespace {
static constexpr int VERBOSE = 0;
static constexpr int DEBUG = 1;

std::string stringFromSockaddr(sockaddr* address) {
    char buffer[INET6_ADDRSTRLEN] = { 0 };

    inet_ntop(address->sa_family, &reinterpret_cast<sockaddr_in*>(address)->sin_addr, buffer, INET6_ADDRSTRLEN);

    return buffer;
}

std::string stringFromErrno(int errNumber) {
    char buffer[1024];
#ifdef _GNU_SOURCE
    // Because GNU != posix, it may choose to return a statically allocated string. In this case,
    // we re-create POSIX-ish semantics and copy the result back into our buffer.

    const char* result = strerror_r(errNumber, buffer, sizeof(buffer));
    if (result != buffer) {
        strncpy(buffer, result, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = 0;
    }
#else
    strerror_r(errNumber, buffer, sizeof(buffer));
#endif
    return buffer;
}

#ifdef __APPLE__
void getStatistics(const ifaddrs* interface, hal::NetworkInterfaceStatistics& statistics) {
    const if_data* data = reinterpret_cast<const if_data*>(interface->ifa_data);

    if (data) {
        statistics.set_receivebytes(data->ifi_ibytes);
        statistics.set_receivepackets(data->ifi_ipackets);
        statistics.set_receiveerrors(data->ifi_ierrors);
        statistics.set_transmitbytes(data->ifi_obytes);
        statistics.set_transmitpackets(data->ifi_opackets);
        statistics.set_transmiterrors(data->ifi_oerrors);
    }
}
#elif defined(__linux__)

void getStatistics(const ifaddrs* interface, hal::NetworkInterfaceStatistics& statistics) {
    const rtnl_link_stats* stats = reinterpret_cast<const rtnl_link_stats*>(interface->ifa_data);

    if (stats) {
        statistics.set_receive_bytes(stats->rx_bytes);
        statistics.set_receive_packets(stats->rx_packets);
        statistics.set_receive_errors(stats->rx_errors);
        statistics.set_transmit_bytes(stats->tx_bytes);
        statistics.set_transmit_packets(stats->tx_packets);
        statistics.set_transmit_errors(stats->tx_errors);
    }
}

#endif

#ifdef __APPLE__
constexpr bool hasInterfaceStatistics(ifaddrs* interface) { return AF_LINK == interface->ifa_addr->sa_family; }
#elif defined(__linux__)

constexpr bool hasInterfaceStatistics(ifaddrs* interface) { return AF_PACKET == interface->ifa_addr->sa_family; }

#endif
}

namespace hal {
namespace details {

    ::google::protobuf::Map<std::string, hal::NetworkInterfaceStatistics> getInterfaceStatistics() {
        ifaddrs* interfaces = nullptr;
        if (0 != getifaddrs(&interfaces)) {
            LOG(ERROR) << "Call to getifaddrs failed: " << stringFromErrno(errno);
            throw std::runtime_error("Call to getifaddrs failed");
        }

        google::protobuf::Map<std::string, hal::NetworkInterfaceStatistics> perInterfaceData;

        for (ifaddrs* interface = interfaces; interface; interface = interface->ifa_next) {
            if (interface->ifa_addr && hasInterfaceStatistics(interface)) {
                auto iter = perInterfaceData.find(interface->ifa_name);

                if (perInterfaceData.end() == iter) {
                    iter = perInterfaceData
                               .insert(google::protobuf::MapPair<std::string, hal::NetworkInterfaceStatistics>(
                                   std::string(interface->ifa_name)))
                               .first;
                }

                hal::NetworkInterfaceStatistics& statistics = iter->second;
                getStatistics(interface, statistics);
            }

            if (interface->ifa_addr && (AF_INET == interface->ifa_addr->sa_family || AF_INET6 == interface->ifa_addr->sa_family)) {
                auto iter = perInterfaceData.find(interface->ifa_name);

                if (perInterfaceData.end() == iter) {
                    iter = perInterfaceData
                               .insert(google::protobuf::MapPair<std::string, hal::NetworkInterfaceStatistics>(
                                   std::string(interface->ifa_name)))
                               .first;
                }

                hal::NetworkInterfaceStatistics& statistics = iter->second;

                std::string address = stringFromSockaddr(interface->ifa_addr);
                VLOG(DEBUG) << "Interface [" << iter->first << " has address [" << address << "]";

                if (AF_INET == interface->ifa_addr->sa_family) {
                    statistics.add_ipv4_addresses(address);
                } else {
                    statistics.add_ipv6_addresses(address);
                }
            }
        }

        freeifaddrs(interfaces);

        google::protobuf::Map<std::string, hal::NetworkInterfaceStatistics> filteredPerInterfaceData;
        for (auto i = perInterfaceData.begin(); i != perInterfaceData.end(); ++i) {
            if (i->second.ipv4_addresses_size() > 0 || i->second.ipv6_addresses_size() > 0) {
                filteredPerInterfaceData[i->first] = i->second;
            }
        }

        return filteredPerInterfaceData;
    }
}
}
