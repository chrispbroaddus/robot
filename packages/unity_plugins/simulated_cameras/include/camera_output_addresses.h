#pragma once

#include <string>

namespace unity_plugins {

///
/// @brief Publisher addresses for the different image output types
///
struct CameraOutputAddresses {
    std::string image; // visible image address
    std::string depth; // depth image address
    std::string pointcloud; // pointcloud image address
};

} // namespace unity_plugins
