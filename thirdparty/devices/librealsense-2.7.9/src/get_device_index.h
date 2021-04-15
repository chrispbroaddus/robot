#pragma once

#include <cstdint>

// Taken from src/media/ros/ros_file_format.h as of 2.7.9 @ d7ff30b30ea1ab4c4c1f0a8f515127be753f6804
constexpr uint32_t get_device_index() {
    return 0; //TODO: change once SDK file supports multiple devices
}
