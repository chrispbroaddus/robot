
#pragma once

#include "camera_device_interface.h"
#include "packages/hal/include/device_factory.h"
#include "packages/hal/proto/camera_sample.pb.h"

#include <memory>

namespace hal {

class CameraDevice : public CameraDeviceInterface {
public:
    CameraDevice(const hal::details::property_map_t& config)
        : m_config(config) {
        m_camera = DeviceRegistry<CameraDeviceInterface>::instance().create(config);
    }
    ~CameraDevice() = default;

    bool capture(CameraSample& cameraSample) { return m_camera->capture(cameraSample); }

    bool setAutoExposureRoi(float xFraction, float yFraction, float radiusFraction) {
        return m_camera->setAutoExposureRoi(xFraction, yFraction, radiusFraction);
    }

private:
    const hal::details::property_map_t m_config;
    std::shared_ptr<CameraDeviceInterface> m_camera;
};
}
