
#include "packages/hal/include/drivers/joysticks/sdl2/joystick_device.h"
#include <iostream>
#include <stdexcept>

#include "SDL.h"
#include "glog/logging.h"

namespace hal {

struct SDLJoystickState {
    SDLJoystickState()
        : joystick(nullptr) {}

    ~SDLJoystickState() {
        if (SDL_JoystickGetAttached(joystick)) {
            SDL_JoystickClose(joystick);
            SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        }
    }

    SDL_Joystick* joystick;
};

JoystickDevice::JoystickDevice(const size_t joystickDeviceIndex)
    : m_dataReady(true) {

    LOG(INFO) << "Creating SDL joystick on device index: " << joystickDeviceIndex;

    m_sdlJoystickState = std::unique_ptr<SDLJoystickState>(new SDLJoystickState());

    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0) {
        throw std::runtime_error("Unable to initialize joystick");
    }

    LOG(INFO) << "Number of joysticks connected: " << (int)SDL_NumJoysticks();

    if (SDL_NumJoysticks() > 0) {
        if ((int)joystickDeviceIndex > SDL_NumJoysticks()) {
            throw std::runtime_error("Joystick index out of range");
        }
        m_sdlJoystickState->joystick = SDL_JoystickOpen(joystickDeviceIndex);
        if (m_sdlJoystickState->joystick == nullptr) {
            LOG(ERROR) << "ERROR: Unable to open joystick";
            throw std::runtime_error("Unable to open joystick");
        }

        m_deviceName = std::string(SDL_JoystickNameForIndex(joystickDeviceIndex));
        LOG(INFO) << "Connected to joystick: " << m_deviceName;
    } else {
        LOG(ERROR) << "ERROR: No joysticks connected";
        throw std::runtime_error("ERROR: Unable to open joystick");
    }

    LOG(INFO) << "Successfully created SDL joystick";

    m_timer.reset();
}

JoystickDevice::~JoystickDevice() {}

bool JoystickDevice::poll() {
    if (m_dataReady) {
        return true;
    } else {
        if (m_timer.elapsed() > static_cast<double>(1 / m_pollFrequency)) {
            m_timer.reset();
            m_dataReady = true;
        }
        return m_dataReady;
    }
}

bool JoystickDevice::capture(hal::JoystickSample& joystickSample) {
    if (!m_dataReady) {
        return false;
    }

    SDL_JoystickUpdate();

    const int numAxes = SDL_JoystickNumAxes(m_sdlJoystickState->joystick);

    LOG(INFO) << "Capturing joystick input with " << numAxes << " axes";

    if (numAxes < 0) {
        LOG(ERROR) << "SDL failed to get number of axes: " << SDL_GetError();
        return false;
    } else {
        // Device
        hal::Device* device = new hal::Device();
        CHECK_NOTNULL(device);
        device->set_name(deviceName());
        device->set_serialnumber(0);

        // HardwareTimestamp
        core::HardwareTimestamp* hardwareTimestamp = new core::HardwareTimestamp();
        CHECK_NOTNULL(hardwareTimestamp);
        hardwareTimestamp->set_nanos(0);

        // SystemTimestamp
        core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
        CHECK_NOTNULL(systemTimestamp);
        systemTimestamp->set_nanos(0);

        for (int axisIndex = 0; axisIndex < numAxes; axisIndex++) {
            const int16_t axisValue = SDL_JoystickGetAxis(m_sdlJoystickState->joystick, axisIndex);

            // See SDL2 documentation on axis limits.
            // https://wiki.libsdl.org/SDL_JoystickGetAxis
            float normalizedAxisValue;
            if (axisValue < 0) {
                normalizedAxisValue = (float)axisValue / 32768.f;
            } else {
                normalizedAxisValue = (float)axisValue / 32767.f;
            }

            joystickSample.add_axis(normalizedAxisValue);
        }

        m_dataReady = false;

        return true;
    }
}

} // namespace hal
