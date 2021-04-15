#include "packages/hal/include/drivers/cameras/unity_simulated/unity_simulated_camera_device_factory.h"

namespace hal {

std::shared_ptr<CameraDeviceInterface> UnitySimulatedCameraDeviceFactory::create(const hal::details::property_map_t& config) {

    CameraId cameraId;

    auto iter = config.find("cameraId");
    if (iter != config.end()) {
        CameraId_Parse(iter->second, &cameraId);
    } else {
        LOG(ERROR) << "Missing property: cameraId";
        throw std::runtime_error("Missing property: cameraId");
    }

    LOG(INFO) << "Unity simulated camera being created";
    return std::make_shared<UnitySimulatedCameraDevice>(cameraId, config);
}
}
