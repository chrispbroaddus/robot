#pragma once

#include <cstdint>

namespace docking {

///
/// The data representing telemetry data from VCU required for inverse kinematic control
///
struct VcuIkTelemetryState {
    float leftServoAngle;
    uint64_t timestampLeftServoAngle = 0;
    float rightServoAngle;
    uint64_t timestampRightServoAngle = 0;
    float leftServoHeight;
    uint64_t timestampLeftServoHeight = 0;
    float rightServoHeight;
    uint64_t timestampRightServoHeight = 0;
};
}