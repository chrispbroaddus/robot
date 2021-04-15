#include "packages/hal/include/drivers/cameras/depth/realsense_driver_factory.h"
#include "gtest/gtest.h"

using namespace hal;

TEST(RealsenseDriverFactoryTest, canInstantiate) { EXPECT_NO_THROW(RealsenseDriverFactory s_RealsenseDriverFactory()); }

TEST(RealsenseDriverFactoryTest, noMode) {
    RealsenseDriverFactory realsenseDriverFactory;
    hal::details::property_map_t deviceConfig;
    EXPECT_THROW(realsenseDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(RealsenseDriverFactoryTest, badMode) {
    RealsenseDriverFactory realsenseDriverFactory;
    hal::details::property_map_t deviceConfig;
    deviceConfig["mode"] = "RANDOM";
    EXPECT_THROW(realsenseDriverFactory.create(deviceConfig), std::runtime_error);
}
