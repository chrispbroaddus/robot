#pragma once

#include "packages/core/proto/timestamp.pb.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"

#include "dc1394/dc1394.h"

namespace hal {

class Dc1394Driver : public CameraDeviceInterface {
public:
    Dc1394Driver(const uint64_t& guid, // ToDo: Replace with serial number
        const dc1394video_mode_t& videoMode, const dc1394color_coding_t& colorCoding, const uint32_t& left, const uint32_t& top,
        const uint32_t& width, const uint32_t& height, const float& fps, const uint32_t& nDma, const bool externalTrigger);
    ~Dc1394Driver();
    std::string deviceName() const override;
    uint64_t serialNumber() const override;
    bool capture(CameraSample& cameraSample) override;
    bool setAutoExposureRoi(float /*xFraction*/, float /*yFraction*/, float /*radiusFraction*/) { return false; }

private:
    dc1394_t* bus;
    dc1394camera_t* camera;
    uint32_t imageWidth;
    uint32_t imageHeight;
    uint32_t bytesPerPixel;
    uint32_t frameCount;
};
}
