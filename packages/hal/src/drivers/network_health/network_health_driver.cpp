#include "packages/hal/include/drivers/network_health/network_health_driver.h"
#include "packages/core/include/chrono.h"
#include "packages/hal/include/drivers/network_health/interface_statistics.h"
#include "packages/hal/include/drivers/network_health/pingv4.h"

#include "glog/logging.h"

#include <thread>

namespace {
template <size_t NUM_ATTEMPTS, typename CLOCK_TYPE>
hal::NetworkPingStatistics networkPingStatistics(
    const std::pair<hal::details::ResolveStatistics<CLOCK_TYPE>, hal::details::PingStatistics<NUM_ATTEMPTS, CLOCK_TYPE> >& results) {
    int count = 0;
    double mean = 0;
    double variance = 0;
    double maximum = 0;

    hal::NetworkPingStatistics result;
    result.set_resolve_attempts(results.first.unsuccessfulAttempts + results.first.successfulAttempts);
    result.set_resolve_successes(results.first.successfulAttempts);
    result.set_target_host(results.first.targetHost);
    result.set_ping_transmit(results.second.echoRequestsSent);
    result.set_ping_receive(results.second.echoRequestsReceived);

    for (const auto& x : results.second.roundTripTimes) {
        if (x.max() != x) {
            std::chrono::duration<double> rttSeconds(x);
            maximum = std::max(maximum, rttSeconds.count());
            const auto prevMean = mean;
            mean += (rttSeconds.count() - mean) / ++count;
            variance += (rttSeconds.count() - mean) * (rttSeconds.count() - prevMean);
        }
    }

    if (count > 1) {
        variance /= (count - 1);
    }

    result.set_average_round_trip_time_seconds(mean);
    result.set_maximum_round_trip_time_seconds(maximum);
    result.set_variance_round_trip_time_seconds(variance);

    return result;
}
}

namespace hal {
NetworkHealthTelemetry NetworkHealthDriver::sample() {
    NetworkHealthTelemetry result;

    try {
        result.mutable_measurement_start_system_timestamp()->set_nanos(core::chrono::gps::wallClockInNanoseconds().count());
        *result.mutable_per_interface_statistics() = details::getInterfaceStatistics();
        *result.mutable_canonical_host_hame() = boost::asio::ip::host_name();

        boost::asio::io_service service;
        boost::asio::deadline_timer timer(service);
        timer.expires_from_now(boost::posix_time::seconds(120));
        timer.async_wait([](const boost::system::error_code&) {});

        // Run the IO service in a background thread
        std::thread worker([&service]() { service.run(); });

        hal::details::PingV4 pingV4(service, targets);
        const auto pingResults = pingV4.ping();

        timer.cancel();
        worker.join();

        *result.mutable_ipv4_statistics() = networkPingStatistics(pingResults);
        result.mutable_measurement_end_system_timestamp()->set_nanos(core::chrono::gps::wallClockInNanoseconds().count());
    } catch (const std::exception& e) {
        LOG(ERROR) << "Something went wrong when attempting to sample network health [" << e.what() << "]. Sending what we have anyway.";
    }
    return result;
}
}