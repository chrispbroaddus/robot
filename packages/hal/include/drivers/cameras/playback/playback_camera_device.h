#pragma once

#include "packages/hal/include/drivers/cameras/camera_device_interface.h"
#include "packages/image_codec/include/image_decoder_interface.h"
#include "packages/serialization/include/protobuf_io.h"

#include <fstream>

namespace hal {

class PlaybackCameraDevice : public CameraDeviceInterface {
public:
    PlaybackCameraDevice(const std::string& path, uint32_t frameDelayInMilliseconds);
    ~PlaybackCameraDevice() = default;

    std::string deviceName() const override { return "PlaybackCameraDevice"; }

    uint64_t serialNumber() const override { return 0; }

    bool capture(CameraSample& cameraSample) override;
    bool setAutoExposureRoi(float /*xFraction*/, float /*yFraction*/, float /*radiusFraction*/) { return false; }

private:
    const std::string m_path;
    const uint32_t m_frameDelayInMilliseconds;
    std::vector<std::string> m_files;
    size_t m_fileIndex;
    std::ifstream m_inputFileStream;
    std::unique_ptr<serialization::ProtobufReader> m_protobufReader;
    std::unique_ptr<image_codec::ImageDecoder> m_decoder;
};
}
