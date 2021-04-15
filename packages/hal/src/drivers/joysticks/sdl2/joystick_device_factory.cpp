
#include "packages/hal/include/drivers/joysticks/sdl2/joystick_device_factory.h"
#include "packages/hal/include/drivers/joysticks/sdl2/joystick_device.h"
#include "packages/hal/include/string_utils.h"

namespace hal {

std::shared_ptr<JoystickDeviceInterface> JoystickDeviceFactory::create(const hal::details::property_map_t& config) {
    LOG(INFO) << "JoystickDevice being created";

    uint32_t deviceIndex;
    auto endIter = config.end();
    auto iter = config.find("deviceIndex");
    if (iter != endIter) {
        deviceIndex = lexicalCast<uint32_t>(iter->second);
    } else {
        throw std::runtime_error("Missing property: deviceIndex");
    }

    return std::make_shared<JoystickDevice>(deviceIndex);
}
}