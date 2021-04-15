#include "packages/core/include/chrono.h"
#include "gtest/gtest.h"
#include <thread>

TEST(wallClockInNanoseconds, positivityTest) {
    auto duration = core::chrono::gps::wallClockInNanoseconds();
    ASSERT_TRUE(duration.count() > 0);
}

TEST(wallClockInNanoseconds, countingSecondsTest) {
    for (size_t i = 0; i < 3; i++) {
        std::chrono::nanoseconds beginNanoseconds = core::chrono::gps::wallClockInNanoseconds();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::chrono::nanoseconds endNanoseconds = core::chrono::gps::wallClockInNanoseconds();
        double diffInSeconds = (endNanoseconds.count() - beginNanoseconds.count()) / 1e9;
        EXPECT_GE(diffInSeconds, 0.8);
        EXPECT_LE(diffInSeconds, 1.2);
    }
}
