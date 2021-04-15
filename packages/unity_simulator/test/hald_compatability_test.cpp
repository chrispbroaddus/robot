#include "glog/logging.h"
#include "gtest/gtest.h"

#include <atomic>
#include <cmath>
#include <thread>

#include "packages/hald/include/hald_application.h"
#include "packages/net/include/zmq_topic_sub.h"

namespace {
constexpr auto DEFAULT_HALD_CONFIG_FILE = "config/11111111-2222-3333-4444-555555555555/hald.conf";
}
void runHald() {

    auto config = hald::parseConfig(DEFAULT_HALD_CONFIG_FILE);
    hald::HaldApplication haldApplication(config);

    int cnt = 0;
    while (haldApplication.isRunning() && cnt < 5) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cnt++;
    }
}

///
/// @brief Check if hald does not throw exception with the default config/.../hald.conf file.
///
TEST(HaldCompatabilityTest, noThrow) { EXPECT_NO_THROW(runHald()); }
