#pragma once

namespace unity_plugins {

///
/// @brief Interop struct for holding the stats data from a frame
///
struct SimulatorStats {
    float deltaTimeMs; // Delta time between frames in milliseconds
    int frameNumber; // Current frame number
};
} // namespace unity_plugins
