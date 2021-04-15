#pragma once

#include <cstdint>
#include <string>

namespace hal {
namespace details {

    template <typename CLOCK_TYPE> struct ResolveStatistics {
        using time_point = typename CLOCK_TYPE::time_point;
        using duration = typename CLOCK_TYPE::duration;

        inline ResolveStatistics()
            : targetHost()
            , targetAddress()
            , maximumAttempts(0)
            , unsuccessfulAttempts(0)
            , successfulAttempts(0)
            , resolveStart(time_point::max())
            , resolveDuration(duration::max()) {}

        std::string targetHost;
        std::string targetAddress;
        uint32_t maximumAttempts;
        uint32_t unsuccessfulAttempts;
        uint32_t successfulAttempts;
        time_point resolveStart;
        duration resolveDuration;

        void clear() {
            targetHost = targetAddress = "";
            maximumAttempts = unsuccessfulAttempts = successfulAttempts = 0;
            resolveStart = time_point::max();
            resolveDuration = duration::max();
        }
    };
}
}
