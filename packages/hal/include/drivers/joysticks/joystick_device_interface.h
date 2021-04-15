
#pragma once

#include "packages/hal/include/device_interface.h"
#include "packages/hal/proto/joystick_sample.pb.h"

namespace hal {

class JoystickDeviceInterface : public DeviceInterface {
public:
    JoystickDeviceInterface() = default;
    ~JoystickDeviceInterface() = default;

    virtual bool poll() = 0;
    virtual bool capture(hal::JoystickSample& joystickSample) = 0;
};
}