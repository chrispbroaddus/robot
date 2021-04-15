#pragma once

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"
#include "packages/hal/include/drivers/cameras/flycapture/flycapture_driver.h"

namespace hal {

class FlycaptureDriverFactory : public DeviceFactory<CameraDeviceInterface> {
public:
    FlycaptureDriverFactory()
        : DeviceFactory<CameraDeviceInterface>(factoryName()) {}
    ~FlycaptureDriverFactory() = default;

    static std::string factoryName() { return "FlycaptureCamera"; }

    std::shared_ptr<CameraDeviceInterface> create(const hal::details::property_map_t& config) override;
};
}
