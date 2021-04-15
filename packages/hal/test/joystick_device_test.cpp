
#include "gtest/gtest.h"

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/device_registry.h"
#include "packages/hal/include/factory_registration.h"

#include "packages/hal/include/drivers/joysticks/joystick_device_interface.h"
#include "packages/hal/proto/joystick_sample.pb.h"

#include <thread>

using namespace hal;

TEST(Joystick, CreateSDLJoystickDevice) {

    hal::Initialize();

    hal::details::property_map_t deviceConfig;
    deviceConfig[hal::details::FACTORY_NAME_KEY] = "SDL2Joystick";
    deviceConfig["deviceIndex"] = std::to_string(0);

    std::shared_ptr<JoystickDeviceInterface> joystick;

    EXPECT_NO_THROW({ joystick = DeviceRegistry<JoystickDeviceInterface>::instance().create(deviceConfig); });

    EXPECT_TRUE(joystick.get());
}

TEST(Joystick, canCaptureValidSample) {

    hal::Initialize();

    JoystickSample joystickSample;
    hal::details::property_map_t deviceConfig;
    deviceConfig[hal::details::FACTORY_NAME_KEY] = "SDL2Joystick";
    deviceConfig["deviceIndex"] = std::to_string(0);

    std::shared_ptr<JoystickDeviceInterface> joystick = DeviceRegistry<JoystickDeviceInterface>::instance().create(deviceConfig);

    EXPECT_NO_THROW(joystick->capture(joystickSample));
    EXPECT_TRUE(joystickSample.axis_size());
}
