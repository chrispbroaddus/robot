#include "glog/logging.h"
#include "packages/core/include/chrono.h"

extern "C" {
///
/// @brief Get the gps time in nanoseconds
///
uint64_t SimulatorTime_gpsSystemtimeCount() {
    auto gpsSystemtime = core::chrono::gps::wallClockInNanoseconds();
    uint64_t gpsSystemtimeCount = gpsSystemtime.count();
    return gpsSystemtimeCount;
}

///
/// @brief Add time in seconds to an existing gps time in nanoseconds and return the result
///
uint64_t SimulatorTime_addSeconds(uint64_t gpsSystemtimeCount, float seconds) {
    std::chrono::duration<double> deltaSeconds(seconds);
    auto deltaNanoSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(deltaSeconds);
    return gpsSystemtimeCount + deltaNanoSeconds.count();
}
}