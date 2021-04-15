
#pragma once

#include "packages/hal/include/drivers/gps/gps_device_interface.h"
#include <gps.h>

namespace hal {

/// This is a device driver that captures GPS data from the GPS daemon. It assumes that
/// gpsd has been started and publishing information on a TCP/IP port.
class GPSDDevice : public GPSDeviceInterface {
public:
    GPSDDevice();
    ~GPSDDevice();

    std::string deviceName() const override { return m_deviceName; }
    uint64_t serialNumber() const override { return 0; }

    bool poll();
    bool capture(hal::GPSTelemetry& telemetry) override;

private:
    std::string m_deviceName;
    struct gps_data_t m_gpsData;

    /// The time epoch provided by gpsd is Unix. Here we need to convert this time duration
    /// into GPS epoch which is the specification for the timestamp values in the interface.
    uint64_t convertTimestamp(const double unixTimestamp);
};
}
