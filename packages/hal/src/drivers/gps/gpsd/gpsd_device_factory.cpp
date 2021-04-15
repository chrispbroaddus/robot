
#include "packages/hal/include/drivers/gps/gpsd/gpsd_device_factory.h"
#include "packages/hal/include/drivers/gps/gpsd/gpsd_device.h"

using namespace hal;

std::shared_ptr<GPSDeviceInterface> GPSDDeviceFactory::create(const hal::details::property_map_t& /*config*/) {
    return std::make_shared<GPSDDevice>();
}
