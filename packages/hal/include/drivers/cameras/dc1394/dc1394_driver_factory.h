#pragma once

#include "dc1394_driver.h"
#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"

namespace hal {

class DC1394DriverFactory : public DeviceFactory<CameraDeviceInterface> {
public:
    DC1394DriverFactory()
        : DeviceFactory<CameraDeviceInterface>(factoryName()) {}
    ~DC1394DriverFactory() = default;

    static std::string factoryName() { return "Dc1394Camera"; }

    std::shared_ptr<CameraDeviceInterface> create(const hal::details::property_map_t& config) override;
};
}
