#pragma once

#include "packages/unity_plugins/utils/include/vector3.h"

namespace unity_plugins {

struct IMUReading {
    Vector3 accel;
    Vector3 gyro;
    uint64_t gpsSystemtimeCount; // gps time in nanoseconds
};

} // namespace unity_plugins
