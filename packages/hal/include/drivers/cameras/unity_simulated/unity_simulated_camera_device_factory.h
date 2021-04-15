#pragma once

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"
#include "unity_simulated_camera_device.h"

namespace hal {

class UnitySimulatedCameraDeviceFactory : public DeviceFactory<CameraDeviceInterface> {
public:
    UnitySimulatedCameraDeviceFactory()
        : DeviceFactory<CameraDeviceInterface>(factoryName()) {}
    ~UnitySimulatedCameraDeviceFactory() = default;

    static std::string factoryName() { return "UnitySimulatedCamera"; }

    std::shared_ptr<CameraDeviceInterface> create(const hal::details::property_map_t& config) override;
};
}
