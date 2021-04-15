
#pragma once

#include "packages/hal/include/drivers/joysticks/joystick_device_interface.h"
#include "packages/hal/include/timer.h"

namespace hal {

struct SDLJoystickState;

class JoystickDevice : public JoystickDeviceInterface {
public:
    JoystickDevice(const size_t joystickDeviceIndex);
    ~JoystickDevice();

    std::string deviceName() const override { return m_deviceName; }

    uint64_t serialNumber() const override { return 0; }

    bool poll();
    bool capture(hal::JoystickSample& joystickSample) override;

private:
    std::string m_deviceName;
    std::unique_ptr<SDLJoystickState> m_sdlJoystickState;
    hal::Timer m_timer;
    bool m_dataReady;
    static constexpr float m_pollFrequency = 100;
};
}