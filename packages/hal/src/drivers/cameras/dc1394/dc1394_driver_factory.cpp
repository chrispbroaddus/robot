#include "packages/hal/include/drivers/cameras/dc1394/dc1394_driver_factory.h"
#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"
#include "packages/hal/include/drivers/cameras/dc1394/dc1394_driver.h"
#include "packages/hal/include/string_utils.h"

#include <dc1394/dc1394.h>
#include <iostream>

namespace hal {

std::shared_ptr<CameraDeviceInterface> DC1394DriverFactory::create(const hal::details::property_map_t& config) {
    uint64_t guid;
    dc1394video_mode_t videoMode;
    uint32_t left;
    uint32_t top;
    uint32_t width;
    uint32_t height;
    uint32_t nDma;
    float fps;
    dc1394color_coding_t colorCoding;
    bool externalTrigger;

    std::string sVideoMode;
    std::string sColorCode;
    std::string sExternalTrigger;
    auto endIter = config.end();

    auto iter = config.find("guid");
    if (iter != endIter) {
        guid = lexicalCast<uint64_t>(iter->second);
    } else {
        LOG(ERROR) << "Missing property: guid";
        throw std::runtime_error("Missing property: guid");
    }

    iter = config.find("width");
    if (iter != endIter) {
        width = lexicalCast<uint32_t>(iter->second);
    } else {
        LOG(ERROR) << "Missing property: width";
        throw std::runtime_error("Missing property: width");
    }

    iter = config.find("height");
    if (iter != endIter) {
        height = lexicalCast<uint32_t>(iter->second);
    } else {
        LOG(ERROR) << "Missing property: height";
        throw std::runtime_error("Missing property: height");
    }

    iter = config.find("left");
    if (iter != endIter) {
        left = lexicalCast<uint32_t>(iter->second);
    } else {
        LOG(ERROR) << "Missing property: left";
        throw std::runtime_error("Missing property: left");
    }

    iter = config.find("top");
    if (iter != endIter) {
        top = lexicalCast<uint32_t>(iter->second);
    } else {
        LOG(ERROR) << "Missing property: top";
        throw std::runtime_error("Missing property: top");
    }

    iter = config.find("nDma");
    if (iter != endIter) {
        nDma = lexicalCast<uint32_t>(iter->second);
    } else {
        LOG(ERROR) << "Missing property: nDma";
        throw std::runtime_error("Missing property: nDma");
    }

    iter = config.find("fps");
    if (iter != endIter) {
        fps = lexicalCast<float>(iter->second);
    } else {
        LOG(ERROR) << "Missing property: fps";
        throw std::runtime_error("Missing property: fps");
    }

    iter = config.find("mode");
    if (iter != endIter) {
        sVideoMode = lexicalCast<std::string>(iter->second);
    } else {
        LOG(ERROR) << "Missing property: mode";
        throw std::runtime_error("Missing property: mode");
    }

    iter = config.find("color");
    if (iter != endIter) {
        sColorCode = lexicalCast<std::string>(iter->second);
    } else {
        LOG(ERROR) << "Missing property: color code";
        throw std::runtime_error("Missing property: color code");
    }

    iter = config.find("externalTrigger");
    if (iter != endIter) {
        sExternalTrigger = lexicalCast<std::string>(iter->second);
    } else {
        LOG(ERROR) << "Missing property: external trigger";
        throw std::runtime_error("Missing property: external trigger");
    }

    if (sVideoMode == "FORMAT7_0") {
        videoMode = DC1394_VIDEO_MODE_FORMAT7_0;
    } else if (sVideoMode == "FORMAT7_1") {
        videoMode = DC1394_VIDEO_MODE_FORMAT7_1;
    } else if (sVideoMode == "FORMAT7_2") {
        videoMode = DC1394_VIDEO_MODE_FORMAT7_2;
    } else if (sVideoMode == "FORMAT7_5") {
        videoMode = DC1394_VIDEO_MODE_FORMAT7_5;
    } else {
        LOG(ERROR) << "Unsupported Video Mode";
        throw std::runtime_error("Unsupported Video Mode");
    }

    if (sColorCode == "MONO8") {
        colorCoding = DC1394_COLOR_CODING_MONO8;
    } else if (sColorCode == "RAW8") {
        colorCoding = DC1394_COLOR_CODING_RAW8;
    } else if (sColorCode == "RGB8") {
        colorCoding = DC1394_COLOR_CODING_RGB8;
    } else {
        LOG(ERROR) << "Unsupported Color Code";
        throw std::runtime_error("Unsupported Color Code");
    }

    if (sExternalTrigger == "ON") {
        externalTrigger = true;
    } else if (sExternalTrigger == "OFF") {
        externalTrigger = false;
    } else {
        LOG(ERROR) << "Invalid External Trigger setting";
        throw std::runtime_error("Invalid External Trigger setting");
    }

    return std::make_shared<Dc1394Driver>(guid, videoMode, colorCoding, left, top, width, height, fps, nDma, externalTrigger);
}
}
