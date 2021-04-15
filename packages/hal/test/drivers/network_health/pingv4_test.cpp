#include "packages/hal/include/drivers/network_health/pingv4.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

#include <unistd.h>

namespace {
void requireRunAsRoot() { ASSERT_EQ(0, geteuid()) << "Executable needs root permissions to run successfully"; }
}

TEST(PingV4Test, canInstantiate) {
    requireRunAsRoot();

    std::vector<std::string> targets({ "google.com" });
    boost::asio::io_service service;
    EXPECT_NO_THROW(std::make_shared<hal::details::PingV4>(service, targets));
}

TEST(PingV4Test, constructorFailsOnEmptyList) {
    requireRunAsRoot();

    std::vector<std::string> targets;
    boost::asio::io_service service;
    EXPECT_THROW(std::make_shared<hal::details::PingV4>(service, targets), std::invalid_argument);
}

TEST(PingV4Test, canPingAmazon) {
    requireRunAsRoot();

    std::vector<std::string> targets({ "amazon.com" });
    boost::asio::io_service service;
    hal::details::PingV4 ping(service, targets);

    boost::asio::deadline_timer timer(service);
    timer.expires_from_now(boost::posix_time::seconds(3));
    timer.async_wait([](const boost::system::error_code& ec) { LOG(INFO) << "Unlatching I/O service. " << ec.message(); });

    std::thread worker([&service]() {
        service.run();
        LOG(INFO) << "IO service completed";
    });

    LOG(INFO) << "Starting ping";
    ping.ping();

    timer.cancel();
    service.stop();
    worker.join();
}