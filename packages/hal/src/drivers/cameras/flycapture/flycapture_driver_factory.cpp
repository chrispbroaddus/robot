#include "packages/hal/include/drivers/cameras/flycapture/flycapture_driver_factory.h"
#include "packages/hal/include/string_utils.h"

#include <flycapture/FlyCapture2.h>
#include <iostream>

namespace hal {

template <typename TARGET_TYPE>
void getProperty(const hal::details::property_map_t& config, const std::string& propertName, TARGET_TYPE& target) {

    auto endIter = config.end();
    auto iter = config.find(propertName);
    if (iter != endIter) {
        target = lexicalCast<TARGET_TYPE>(iter->second);
    } else {
        LOG(ERROR) << "Missing property: " + propertName;
        throw std::runtime_error("Missing property: " + propertName);
    }
}

std::shared_ptr<CameraDeviceInterface> FlycaptureDriverFactory::create(const hal::details::property_map_t& config) {

    uint64_t serialNumber;
    uint32_t left;
    uint32_t top;
    uint32_t width;
    uint32_t height;
    float fps;
    uint32_t numBuffers;
    bool externalTrigger;
    FlyCapture2::GrabMode grabMode;
    FlyCapture2::Mode format7Mode;
    FlyCapture2::PixelFormat pixelFormat;
    uint32_t captureTimeoutInMilliseconds;
    uint32_t AERoILeft;
    uint32_t AERoITop;
    uint32_t AERoIWidth;
    uint32_t AERoIHeight;

    getProperty<uint64_t>(config, "serialNumber", serialNumber);
    getProperty<uint32_t>(config, "width", width);
    getProperty<uint32_t>(config, "height", height);
    getProperty<uint32_t>(config, "left", left);
    getProperty<uint32_t>(config, "top", top);
    getProperty<uint32_t>(config, "numBuffers", numBuffers);
    getProperty<float>(config, "fps", fps);
    getProperty<uint32_t>(config, "captureTimeoutInMilliseconds", captureTimeoutInMilliseconds);
    getProperty<uint32_t>(config, "AERoILeft", AERoILeft);
    getProperty<uint32_t>(config, "AERoITop", AERoITop);
    getProperty<uint32_t>(config, "AERoIWidth", AERoIWidth);
    getProperty<uint32_t>(config, "AERoIHeight", AERoIHeight);

    std::string externalTriggerStr;
    getProperty<std::string>(config, "externalTrigger", externalTriggerStr);
    if (externalTriggerStr == "ON") {
        externalTrigger = true;
    } else if (externalTriggerStr == "OFF") {
        externalTrigger = false;
    } else {
        LOG(ERROR) << "Invalid External Trigger setting";
        throw std::runtime_error("Invalid External Trigger setting");
    }

    std::string grabModeStr;
    getProperty<std::string>(config, "grabMode", grabModeStr);
    if (grabModeStr == "DROP_FRAMES") {
        grabMode = FlyCapture2::DROP_FRAMES;
    } else if (grabModeStr == "BUFFER_FRAMES") {
        grabMode = FlyCapture2::BUFFER_FRAMES;
    } else {
        LOG(ERROR) << "Unsupported grabMode";
        throw std::runtime_error("Unsupported grabMode");
    }

    std::string pixelFormatStr;
    getProperty<std::string>(config, "pixelFormat", pixelFormatStr);
    if (pixelFormatStr == "MONO8") {
        pixelFormat = FlyCapture2::PIXEL_FORMAT_MONO8;
    } else if (pixelFormatStr == "RAW8") {
        pixelFormat = FlyCapture2::PIXEL_FORMAT_RAW8;
    } else if (pixelFormatStr == "RGB") {
        pixelFormat = FlyCapture2::PIXEL_FORMAT_RGB;
    } else if (pixelFormatStr == "RGB8") {
        pixelFormat = FlyCapture2::PIXEL_FORMAT_RGB8;
    } else {
        LOG(ERROR) << "Unsupported pixelFormat";
        throw std::runtime_error("Unsupported pixelFormat");
    }

    std::string format7ModeStr;
    getProperty<std::string>(config, "format7Mode", format7ModeStr);
    if (format7ModeStr == "MODE_0") {
        format7Mode = FlyCapture2::MODE_0;
    } else if (format7ModeStr == "MODE_1") {
        format7Mode = FlyCapture2::MODE_1;
    } else if (format7ModeStr == "MODE_2") {
        format7Mode = FlyCapture2::MODE_2;
    } else if (format7ModeStr == "MODE_5") {
        format7Mode = FlyCapture2::MODE_5;
    } else {
        LOG(ERROR) << "Unsupported format7Mode";
        throw std::runtime_error("Unsupported format7Mode");
    }

    return std::make_shared<FlycaptureDriver>(serialNumber, grabMode, format7Mode, pixelFormat, left, top, width, height, fps, numBuffers,
        captureTimeoutInMilliseconds, externalTrigger, AERoILeft, AERoITop, AERoIWidth, AERoIHeight);
}
}
