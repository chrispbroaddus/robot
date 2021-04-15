
#include "packages/hal/include/drivers/cameras/playback/playback_camera_device_factory.h"
#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"
#include "packages/hal/include/drivers/cameras/playback/playback_camera_device.h"
#include "packages/hal/include/string_utils.h"

#include <iostream>

namespace hal {

std::shared_ptr<CameraDeviceInterface> PlaybackCameraDeviceFactory::create(const hal::details::property_map_t& config) {

    std::string path;
    uint32_t frameDelayInMilliseconds;

    auto endIter = config.end();

    auto iter = config.find("path");
    if (iter != endIter) {
        path = iter->second;
    } else {
        LOG(ERROR) << "Missing property: path";
        throw std::runtime_error("Missing property: path");
    }

    iter = config.find("frameDelayInMilliseconds");
    if (iter != endIter) {
        frameDelayInMilliseconds = lexicalCast<uint32_t>(iter->second);
    } else {
        LOG(ERROR) << "Missing property: frameDelayInMilliseconds";
        throw std::runtime_error("Missing property: frameDelayInMilliseconds");
    }

    LOG(INFO) << "Camera playback is loading: " << path;

    return std::make_shared<PlaybackCameraDevice>(path, frameDelayInMilliseconds);
}
}
