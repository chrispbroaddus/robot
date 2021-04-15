
#pragma once

#include <chrono>
#include <cstdint>

namespace core {
namespace chrono {
    namespace gps {

        /// Get the wall clock from the GPS Epoch of January 6th, 1980 midnight from between
        /// January 5th, 1980 and January 6th, 1980.
        ///
        /// References:
        ///
        ///     http://www.cplusplus.com/reference/chrono/system_clock/from_time_t/
        ///     https://confluence.qps.nl/display/KBE/UTC+to+GPS+Time+Correction
        ///
        /// \return Nanoseconds since GPS Epoch
        inline std::chrono::nanoseconds wallClockInNanoseconds() {
            std::tm timeinfo = std::tm();
            timeinfo.tm_year = (1980 - 1900); // year: 1980
            timeinfo.tm_mon = 0; // month: January
            timeinfo.tm_mday = 6; // day: 6th
            std::time_t tt = timegm(&timeinfo); // Using timegm instead of std::mktime to get the time in GMT
            std::chrono::system_clock::time_point tp = std::chrono::system_clock::from_time_t(tt);
            std::chrono::system_clock::duration d = std::chrono::system_clock::now() - tp;
            return std::chrono::duration_cast<std::chrono::nanoseconds>(d);
        }
    }
}
}
