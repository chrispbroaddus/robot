
#include "packages/hal/include/drivers/gps/gpsd/gpsd_device.h"
#include <chrono>
#include <math.h>

#include "glog/logging.h"

using namespace hal;

GPSDDevice::GPSDDevice() {
    const int connectionSuccess = gps_open("localhost", "2947", &m_gpsData);
    if (connectionSuccess < 0) {
        LOG(ERROR) << "Unable to connect to gpsd";
        throw std::runtime_error("Unable to connect to gpsd");
    }
    gps_stream(&m_gpsData, WATCH_ENABLE | WATCH_JSON, NULL);
}

GPSDDevice::~GPSDDevice() {}

bool GPSDDevice::poll() { return gps_waiting(&m_gpsData, 0); }

bool GPSDDevice::capture(hal::GPSTelemetry& telemetry) {
    if (gps_read(&m_gpsData)) {
        core::SystemTimestamp* timestamp = new core::SystemTimestamp();
        timestamp->set_nanos(convertTimestamp(m_gpsData.fix.time));
        telemetry.set_allocated_timestamp(timestamp);

        telemetry.set_latitude(m_gpsData.fix.latitude);
        telemetry.set_longitude(m_gpsData.fix.longitude);
        telemetry.set_altitude(m_gpsData.fix.altitude);
        telemetry.set_geoid_separation(m_gpsData.separation);
        telemetry.set_num_of_satellites_in_view(m_gpsData.satellites_visible);
        telemetry.set_fix_mode((hal::GPSTelemetry_FixMode)m_gpsData.fix.mode);
        return true;
    } else {
        return false;
    }
}

uint64_t GPSDDevice::convertTimestamp(const double unixTimestamp) {
    std::tm unixEpoch = std::tm();
    unixEpoch.tm_year = (1970 - 1900); // year: 1970
    unixEpoch.tm_mon = 0; // month: January
    unixEpoch.tm_mday = 1; // day: 1st
    const std::time_t unixEpochTime = std::mktime(&unixEpoch);
    const std::chrono::system_clock::time_point unixTimePoint = std::chrono::system_clock::from_time_t(unixEpochTime);

    std::tm gpsEpoch = std::tm();
    gpsEpoch.tm_year = (1980 - 1900); // year: 1980
    gpsEpoch.tm_mon = 0; // month: January
    gpsEpoch.tm_mday = 6; // day: 6th
    const std::time_t gpsEpochTime = std::mktime(&gpsEpoch);
    const std::chrono::system_clock::time_point gpsTimePoint = std::chrono::system_clock::from_time_t(gpsEpochTime);

    const std::chrono::system_clock::duration secondsBetweenEpochs = gpsTimePoint - unixTimePoint;

    const double gpsTimeInSeconds = unixTimestamp - std::chrono::duration_cast<std::chrono::seconds>(secondsBetweenEpochs).count();

    const uint64_t integerPartInNanos = ((uint64_t)gpsTimeInSeconds) * 1e9;
    const uint64_t fractionalPartInNanos = (uint64_t)((gpsTimeInSeconds - floor(gpsTimeInSeconds)) * 1e9);

    return integerPartInNanos + fractionalPartInNanos;
}
