#include "packages/hal/include/drivers/vcu/udp_socket/udp_socket_vcu_driver_factory.h"
#include "gtest/gtest.h"

using namespace hal::vcu;

TEST(HalUdpSocketVcuFactoryTest, canInstantiate) { EXPECT_NO_THROW(UdpSocketVcuDriverFactory udpSocketVcuDriverFactory); }

TEST(HalUdpSocketVcuFactoryTest, missingVCUAddress) {

    UdpSocketVcuDriverFactory udpSocketVcuDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["commandPort"] = "10000";
    deviceConfig["telemetryPort"] = "10002";

    EXPECT_THROW(udpSocketVcuDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUdpSocketVcuFactoryTest, missingCommandPort) {

    UdpSocketVcuDriverFactory udpSocketVcuDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["vcuAddress"] = "0.0.0.0";
    deviceConfig["telemetryPort"] = "10002";

    EXPECT_THROW(udpSocketVcuDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUdpSocketVcuFactoryTest, missingTelemetryPort) {

    UdpSocketVcuDriverFactory udpSocketVcuDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["vcuAddress"] = "0.0.0.0";
    deviceConfig["commandPort"] = "10000";

    EXPECT_THROW(udpSocketVcuDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalUdpSocketVcuFactoryTest, canCreateVcu) {

    UdpSocketVcuDriverFactory udpSocketVcuDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["vcuAddress"] = "0.0.0.0";
    deviceConfig["commandPort"] = "10000";
    deviceConfig["telemetryPort"] = "10002";

    EXPECT_NO_THROW(udpSocketVcuDriverFactory.create(deviceConfig));
}
