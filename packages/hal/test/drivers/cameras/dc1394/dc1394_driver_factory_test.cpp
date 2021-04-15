#include "packages/hal/include/drivers/cameras/dc1394/dc1394_driver_factory.h"
#include "gtest/gtest.h"

using namespace hal;

TEST(HalDc1394DriverFactoryTest, canInstantiate) { EXPECT_NO_THROW(DC1394DriverFactory s_Dc1394DriverFactory()); }

TEST(HalDc1394DriverFactoryTest, noGuid) {
    DC1394DriverFactory dc1394DriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["mode"] = "FORMAT7_0";
    deviceConfig["color"] = "MONO8";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["fps"] = std::to_string(60);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_THROW(dc1394DriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalDc1394DriverFactoryTest, noMode) {
    DC1394DriverFactory dc1394DriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["guid"] = std::to_string(49712223537990881);
    deviceConfig["color"] = "MONO8";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["fps"] = std::to_string(60);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_THROW(dc1394DriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalDc1394DriverFactoryTest, badMode) {
    DC1394DriverFactory dc1394DriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["guid"] = std::to_string(49712223537990881);
    deviceConfig["mode"] = "RANDOM";
    deviceConfig["color"] = "MONO8";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["fps"] = std::to_string(60);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_THROW(dc1394DriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalDc1394DriverFactoryTest, noColorCode) {
    DC1394DriverFactory dc1394DriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["guid"] = std::to_string(49712223537990881);
    deviceConfig["mode"] = "FORMAT7_0";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["fps"] = std::to_string(60);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_THROW(dc1394DriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalDc1394DriverFactoryTest, badColorCode) {
    DC1394DriverFactory dc1394DriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["guid"] = std::to_string(49712223537990881);
    deviceConfig["mode"] = "FORMAT7_0";
    deviceConfig["color"] = "RANDOM";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["fps"] = std::to_string(60);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_THROW(dc1394DriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalDc1394DriverFactoryTest, noWidth) {
    DC1394DriverFactory dc1394DriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["guid"] = std::to_string(49712223537990881);
    deviceConfig["mode"] = "FORMAT7_0";
    deviceConfig["color"] = "MONO8";
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["fps"] = std::to_string(60);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_THROW(dc1394DriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalDc1394DriverFactoryTest, noHeight) {
    DC1394DriverFactory dc1394DriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["guid"] = std::to_string(49712223537990881);
    deviceConfig["mode"] = "FORMAT7_0";
    deviceConfig["color"] = "MONO8";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["fps"] = std::to_string(60);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_THROW(dc1394DriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalDc1394DriverFactoryTest, noLeft) {
    DC1394DriverFactory dc1394DriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["guid"] = std::to_string(49712223537990881);
    deviceConfig["mode"] = "FORMAT7_0";
    deviceConfig["color"] = "MONO8";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["fps"] = std::to_string(60);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_THROW(dc1394DriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalDc1394DriverFactoryTest, noTop) {
    DC1394DriverFactory dc1394DriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["guid"] = std::to_string(49712223537990881);
    deviceConfig["mode"] = "FORMAT7_0";
    deviceConfig["color"] = "MONO8";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["fps"] = std::to_string(60);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_THROW(dc1394DriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalDc1394DriverFactoryTest, noDma) {
    DC1394DriverFactory dc1394DriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["guid"] = std::to_string(49712223537990881);
    deviceConfig["mode"] = "FORMAT7_0";
    deviceConfig["color"] = "MONO8";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["fps"] = std::to_string(60);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_THROW(dc1394DriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalDc1394DriverFactoryTest, noFps) {
    DC1394DriverFactory dc1394DriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["guid"] = std::to_string(49712223537990881);
    deviceConfig["mode"] = "FORMAT7_0";
    deviceConfig["color"] = "MONO8";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_THROW(dc1394DriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalDc1394DriverFactoryTest, noExternalTrigger) {
    DC1394DriverFactory dc1394DriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["guid"] = std::to_string(49712223537990881);
    deviceConfig["mode"] = "FORMAT7_0";
    deviceConfig["color"] = "MONO8";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["fps"] = std::to_string(60);

    EXPECT_THROW(dc1394DriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalDc1394DriverFactoryTest, canCreateCamera) {
    DC1394DriverFactory dc1394DriverFactory;

    dc1394_t* bus;
    dc1394error_t err;
    dc1394camera_list_t* list;
    bus = dc1394_new();
    if (!bus) {
        LOG(ERROR) << "Dc1394: Bus creation failed";
        throw std::runtime_error("Dc1394: Bus creation failed");
    }
    err = dc1394_camera_enumerate(bus, &list);
    if (err != DC1394_SUCCESS) {
        LOG(ERROR) << "Dc1394: Camera enumeration failed";
        throw std::runtime_error("DC1394: Camera enumeration failed");
    }
    if (list->num == 0) {
        LOG(ERROR) << "Dc1394: No cameras found";
        throw std::runtime_error("No cameras found");
    }
    uint64_t guid = list->ids[0].guid;

    hal::details::property_map_t deviceConfig;
    deviceConfig["guid"] = std::to_string(guid);
    deviceConfig["mode"] = "FORMAT7_0";
    deviceConfig["color"] = "MONO8";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["fps"] = std::to_string(60);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_NO_THROW(dc1394DriverFactory.create(deviceConfig));
}
