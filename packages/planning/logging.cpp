#include "packages/planning/logging.h"
#include "packages/core/include/chrono.h"

#include <chrono>

#include "glog/logging.h"

namespace planning {

using core::chrono::gps::wallClockInNanoseconds;

core::SystemTimestamp getTime() {
    auto time = wallClockInNanoseconds();
    core::SystemTimestamp stamp;
    stamp.set_nanos(time.count());
    return stamp;
}

} // planning
