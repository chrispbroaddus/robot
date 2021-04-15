#include "synthetic_joystick_publisher.h"

using namespace unity_plugins;

int main(int argc, char** argv) {
    const int portNumber = 6000;
    const int highWaterMark = 1;
    const int lingerPeriodInMilliseconds = 1000;
    const std::string publishAddr("tcp://127.0.0.1:" + std::to_string(portNumber));
    SyntheticJoystickPublisher publisher(publishAddr, highWaterMark, lingerPeriodInMilliseconds);
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}