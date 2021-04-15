
#include "packages/hal/include/drivers/gps/gpsd/gpsd_device.h"
#include "gtest/gtest.h"
#include <thread>

TEST(gpsd, canCaptureData) {
    hal::GPSDDevice gps;

    bool success = false;
    const size_t pollAttempts = 10;

    for (size_t i = 0; i < pollAttempts; i++) {
        if (gps.poll()) {
            hal::GPSTelemetry telemetry;
            EXPECT_TRUE(gps.capture(telemetry));
            success = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    EXPECT_TRUE(success);
}
