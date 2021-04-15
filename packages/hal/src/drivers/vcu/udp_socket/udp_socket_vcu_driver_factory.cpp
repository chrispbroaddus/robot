#include "packages/hal/include/drivers/vcu/udp_socket/udp_socket_vcu_driver_factory.h"
#include "packages/hal/include/drivers/vcu/udp_socket/udp_socket_vcu_driver.h"

#include <iostream>

namespace hal {

namespace vcu {

    std::shared_ptr<VCUDeviceInterface> UdpSocketVcuDriverFactory::create(const hal::details::property_map_t& config) {

        LOG(INFO) << "Udp Socket VCU driver being created";

        std::string vcuAddress;
        int commandPort;
        int telemetryPort;

        auto endIter = config.end();

        auto iter = config.find("commandPort");
        if (iter != endIter) {
            commandPort = std::stoi(iter->second);
        } else {
            throw std::runtime_error("Missing property: commandPort");
        }

        iter = config.find("telemetryPort");
        if (iter != endIter) {
            telemetryPort = std::stoi(iter->second);
        } else {
            throw std::runtime_error("Missing property: telemetryPort");
        }

        iter = config.find("vcuAddress");
        if (iter != endIter) {
            vcuAddress = iter->second;
        } else {
            throw std::runtime_error("Missing property: vcuAddress");
        }

        return std::make_shared<UdpSocketVcuDriver>(vcuAddress, commandPort, telemetryPort);
    }
} // vcu
} // hal
