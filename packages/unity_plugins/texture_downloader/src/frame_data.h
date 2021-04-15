#pragma once

#include "packages/unity_plugins/utils/include/zippy_image_interop.h"

namespace unity_plugins {

////
/// @brief Frame data containing information about the frame and the texture pointer
///
struct FrameData {
    hal::CameraId cameraId;
    ImageType imageType;
    void* texturePtr;
    int width;
    int height;
    int frameNumber;
    float timestamp;
};
} // namespace unity_plugins
