#pragma once

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"
#include "packages/hal/include/drivers/cameras/depth/realsense_driver.h"

namespace hal {

class RealsenseDriverFactory : public DeviceFactory<CameraDeviceInterface> {
public:
    RealsenseDriverFactory()
        : DeviceFactory<CameraDeviceInterface>(factoryName()) {}
    ~RealsenseDriverFactory() = default;

    static std::string factoryName() { return "Realsense"; }

    std::shared_ptr<CameraDeviceInterface> create(const hal::details::property_map_t& config) override;
};
}
