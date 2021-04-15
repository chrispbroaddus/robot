
#pragma once

#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"
#include "packages/hal/include/drivers/cameras/playback/playback_camera_device.h"

namespace hal {

class PlaybackCameraDeviceFactory : public DeviceFactory<CameraDeviceInterface> {
public:
    PlaybackCameraDeviceFactory()
        : DeviceFactory<CameraDeviceInterface>(factoryName()) {}
    ~PlaybackCameraDeviceFactory() = default;

    static std::string factoryName() { return "PlaybackCamera"; }

    std::shared_ptr<CameraDeviceInterface> create(const hal::details::property_map_t& config) override;
};
}
