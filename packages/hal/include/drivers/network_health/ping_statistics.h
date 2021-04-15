#pragma once

#include <array>
#include <cstdint>
namespace hal {
namespace details {
    template <size_t MAXIMUM_ATTEMPTS, typename CLOCK_TYPE> struct PingStatistics {
        using time_point = typename CLOCK_TYPE::time_point;
        using duration = typename CLOCK_TYPE::duration;

        constexpr PingStatistics()
            : echoRequestsSent(0)
            , echoRequestsReceived(0)
            , transmitStart{ { time_point::max() } }
            , roundTripTimes{ { duration::max() } } {}

        uint32_t echoRequestsSent;
        uint32_t echoRequestsReceived;
        std::array<time_point, MAXIMUM_ATTEMPTS> transmitStart;
        std::array<duration, MAXIMUM_ATTEMPTS> roundTripTimes;

        void clear() {
            echoRequestsSent = echoRequestsReceived = 0;
            std::fill(transmitStart.begin(), transmitStart.end(), time_point::max());
            std::fill(roundTripTimes.begin(), roundTripTimes.end(), duration::max());
        }
    };
}
}
