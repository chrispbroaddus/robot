#include "packages/hal/include/drivers/cameras/depth/realsense_driver_factory.h"
#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"
#include "packages/hal/include/drivers/cameras/depth/realsense_driver.h"

#include <iostream>

namespace hal {

std::shared_ptr<CameraDeviceInterface> RealsenseDriverFactory::create(const hal::details::property_map_t& config) {
    RealsenseDriver::Mode mode;
    auto iter = config.find("mode");
    if (iter != config.end()) {
        if (iter->second == "best_quality") {
            mode = RealsenseDriver::Mode::BEST_QUALITY;
        } else if (iter->second == "largest_image") {
            mode = RealsenseDriver::Mode::LARGEST_IMAGE;
        } else if (iter->second == "highest_framerate") {
            mode = RealsenseDriver::Mode::HIGHEST_FRAMERATE;
        } else {
            std::stringstream error;
            error << "Bad mode property: " << iter->second;
            LOG(ERROR) << error.str();
            throw std::runtime_error(error.str());
        }
    } else {
        LOG(ERROR) << "Missing property: mode";
        throw std::runtime_error("Missing property: mode");
    }
    return std::make_shared<RealsenseDriver>(mode);
}

} // hal
