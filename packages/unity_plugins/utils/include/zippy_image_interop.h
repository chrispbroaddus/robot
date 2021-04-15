#pragma once

#include "packages/hal/proto/camera_id.pb.h"
#include "zippy_interface.h"
#include <cstring>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace unity_plugins {

////
/// @brief The Image types that will be returned from the TextureDownloader plugin
///
enum ImageType {
    Greyscale = 0,
    Color = 1,
    Depth = 2,
    Pointcloud = 3,
    // Segmented     //TODO in the future
};

const ImageType ImageType_MIN = ImageType::Greyscale;
const ImageType ImageType_MAX = ImageType::Pointcloud;
const int ImageType_ARRAYSIZE = ImageType_MAX + 1;

////
/// @brief A struct the describes an image and holds the image data
///
struct CameraImage {
    hal::CameraId cameraId; // Camera this image came from
    ImageType imageType; // Type of image data
    void* data; // Pointer to the mapped OpenGL data. DO NOT modify or keep hold of it
    int width; // Width of the image
    int height; // Height of the image
    int bytesPerPixel; // Number of bytes per pixel
    int frameNumber; // Frame number
    float timestamp; // Unity Timestamp of the image
};

///
/// @brief Function pointer used to receive the downloaded images from the texture downloader
/// @description Define a function that conforms to this and return its pointer to C#/Unity, so it can be connected to the texture
/// downloader and act on
/// the images
typedef void(ZIPPY_INTERFACE_API* ProcessImageFuncPtr)(CameraImage cameraImage);

} // namespace unity_plugins
