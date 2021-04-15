
#pragma once

#include "joystick_device_interface.h"
#include "packages/hal/include/device_factory.h"

namespace hal {

class JoystickDevice : public JoystickDeviceInterface {
public:
    JoystickDevice(const hal::details::property_map_t& config)
        : m_config(config) {
        m_joystick = DeviceRegistry<JoystickDeviceInterface>::instance().create(config);
    }
    ~JoystickDevice() = default;

    bool poll() { return m_joystick->poll(); }
    bool capture(hal::JoystickSample& joystickSample) { return m_joystick->capture(joystickSample); }

private:
    const hal::details::property_map_t m_config;
    std::shared_ptr<JoystickDeviceInterface> m_joystick;
};
}