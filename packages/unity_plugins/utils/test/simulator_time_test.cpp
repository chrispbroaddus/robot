#include "glog/logging.h"
#include "packages/core/include/chrono.h"
#include "gtest/gtest.h"

extern "C" uint64_t SimulatorTime_gpsSystemtimeCount();
extern "C" uint64_t SimulatorTime_addSeconds(uint64_t gpsSystemtimeCount, float seconds);

TEST(SimulatorTime, gpsTimeCount) {

    auto refGpsSystemtime = core::chrono::gps::wallClockInNanoseconds();
    auto gpsSystemtimeCount = SimulatorTime_gpsSystemtimeCount();

    uint64_t refGpsSystemtimeCount = refGpsSystemtime.count();

    EXPECT_GT(gpsSystemtimeCount, refGpsSystemtimeCount);
    EXPECT_NEAR(refGpsSystemtimeCount, gpsSystemtimeCount, 1e4);
}

TEST(SimulatorTime, addSeconds) {
    auto gpsSystemtimeCount = SimulatorTime_gpsSystemtimeCount();

    float oneSec = 1.0f;
    uint64_t oneSecInNanoSec = 1e9;

    auto expectedTime = gpsSystemtimeCount + oneSecInNanoSec;
    auto actualTime = SimulatorTime_addSeconds(gpsSystemtimeCount, oneSec);
    EXPECT_EQ(expectedTime, actualTime);
}
