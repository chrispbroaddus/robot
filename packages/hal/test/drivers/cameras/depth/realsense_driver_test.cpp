#include "packages/hal/include/drivers/cameras/depth/realsense_driver.h"
#include "glog/logging.h"
#include "gtest/gtest.h"
#include <chrono>
#include <thread>

using namespace hal;

TEST(RealsenseDriverTest, initialization) { EXPECT_THROW(RealsenseDriver driver(RealsenseDriver::Mode::BEST_QUALITY), std::runtime_error); }

TEST(RealsenseDriverTest, read) {
    RealsenseDriver driver(RealsenseDriver::Mode::BEST_QUALITY);
    auto start = std::chrono::system_clock::now();
    constexpr int kNumFrames = 100;
    for (int i = 0; i < kNumFrames; ++i) {
        hal::CameraSample sample;
        ASSERT_TRUE(driver.capture(sample));
    }
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count();
    LOG(INFO) << "Estimated frequency: " << kNumFrames / static_cast<float>(elapsed);
    constexpr float kMinimumExpectedFrequency = 30.0;
    LOG(INFO) << "Should be above: " << kMinimumExpectedFrequency;
}
