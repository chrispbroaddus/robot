#pragma once

#include "packages/core/proto/timestamp.pb.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"

namespace hal {

class RealsenseDriver : public CameraDeviceInterface {
public:
    enum class Mode { BEST_QUALITY, LARGEST_IMAGE, HIGHEST_FRAMERATE };

    explicit RealsenseDriver(Mode mode);
    ~RealsenseDriver() = default;
    std::string deviceName() const override;
    uint64_t serialNumber() const override;
    bool capture(CameraSample& cameraSample) override;
    bool setAutoExposureRoi(float /* xFraction */, float /* yFraction */, float /* radiusFraction*/) { return false; }

private:
    RealsenseDriver(const RealsenseDriver& driver) = delete;
    RealsenseDriver(const RealsenseDriver&& driver) = delete;
    RealsenseDriver& operator=(const RealsenseDriver& driver) = delete;
    RealsenseDriver& operator=(const RealsenseDriver&& driver) = delete;

    class RealsenseDriverImpl;
    std::shared_ptr<RealsenseDriverImpl> m_impl;
};
}
