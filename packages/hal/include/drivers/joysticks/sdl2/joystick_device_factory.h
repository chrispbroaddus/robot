
#pragma once

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/joysticks/joystick_device_interface.h"

namespace hal {

class JoystickDeviceFactory : public DeviceFactory<JoystickDeviceInterface> {
public:
    JoystickDeviceFactory()
        : DeviceFactory<JoystickDeviceInterface>(factoryName()) {}
    ~JoystickDeviceFactory() = default;

    static std::string factoryName() { return "SDL2Joystick"; }

    std::shared_ptr<JoystickDeviceInterface> create(const hal::details::property_map_t& config) override;
};
}