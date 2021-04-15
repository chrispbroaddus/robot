// ImageSaver.cpp : Defines the exported functions for the DLL application.
//

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "packages/unity_plugins/image_saver/include/image_saver.h"
#include "glog/logging.h"
#include "packages/unity_plugins/utils/include/zippy_image_interop.h"
#include "stb/stb_image_write.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

namespace unity_plugins {

static std::string s_saveDirectory = ""; // Directory to save images to

////
/// @brief Convert an ImageType to a string
///
inline std::string ImageTypeToString(ImageType imageType) {
    switch (imageType) {
    case ImageType::Greyscale:
        return "Greyscale";
    case ImageType::Color:
        return "Color";
    case ImageType::Depth:
        return "Depth";
    case ImageType::Pointcloud:
        return "Pointcloud";
    default:
        return "Unknown";
    }
}

////
/// @brief Set the directory to save the images to
///
void SetSaveDirectory(const std::string& location) {
    s_saveDirectory = std::string(location);
    if (!s_saveDirectory.empty() && s_saveDirectory.back() != '/') {
        s_saveDirectory = s_saveDirectory + "/";
    }
}

////
/// @brief Generate a filename
///
std::string CreateFileName(const CameraImage& cameraImage) {

    std::ostringstream oss;
    if (!s_saveDirectory.empty()) {
        oss << s_saveDirectory;
    }

    std::string cameraName = hal::CameraId_Name(cameraImage.cameraId);
    std::string imageTypeName = ImageTypeToString(cameraImage.imageType);

    std::string ext = "";
    switch (cameraImage.imageType) {
    case ImageType::Greyscale:
    case ImageType::Color:
        ext = "tga";
        break;
    case ImageType::Depth:
        ext = "depth";
        break;
    case ImageType::Pointcloud:
        ext = "pointcloud";
        break;
    default:
        ext = "unknown";
        break;
    }

    oss << cameraName << "-" << imageTypeName << "-" << std::setfill('0') << std::setw(10) << cameraImage.frameNumber << "-"
        << cameraImage.timestamp << "." << ext;

    return oss.str();
}

////
/// @brief Write the floating point image
///
bool WriteFloatImage(const std::string& fileName, const CameraImage& cameraImage) {
    std::ofstream file(fileName.c_str(), std::ios::out | std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(&cameraImage.width), sizeof(int));
        file.write(reinterpret_cast<const char*>(&cameraImage.height), sizeof(int));
        file.write(reinterpret_cast<const char*>(&cameraImage.frameNumber), sizeof(int));
        file.write(reinterpret_cast<const char*>(&cameraImage.timestamp), sizeof(float));
        file.write(reinterpret_cast<const char*>(cameraImage.data), cameraImage.width * cameraImage.height * cameraImage.bytesPerPixel);

        file.close();
        return true;
    }

    return false;
}

////
/// @brief Save the floating point image
///
void SaveFloatImage(const CameraImage& cameraImage) {
    assert(cameraImage.imageType == ImageType::Depth || cameraImage.imageType == ImageType::Pointcloud);
    std::string fileName = CreateFileName(cameraImage);
    if (!WriteFloatImage(fileName, cameraImage)) {
        LOG(ERROR) << "Failed to save float image: " << fileName;
    }
}

////
/// @brief Save the regular image
///
void SaveByteImage(const CameraImage& cameraImage) {
    assert(cameraImage.imageType == ImageType::Color || cameraImage.imageType == ImageType::Greyscale);
    std::string fileName = CreateFileName(cameraImage);

    int result = stbi_write_tga(fileName.c_str(), cameraImage.width, cameraImage.height, cameraImage.bytesPerPixel, cameraImage.data);

    if (!result != 0) {
        LOG(ERROR) << "Failed to save image: " << fileName;
    }
}

void SaveImage(const CameraImage& cameraImage) {
    switch (cameraImage.imageType) {
    case ImageType::Color:
    case ImageType::Greyscale:
        SaveByteImage(cameraImage);
        break;
    case ImageType::Depth:
    case ImageType::Pointcloud:
        SaveFloatImage(cameraImage);
        break;
    default:
        LOG(ERROR) << "Unknown image type";
        break;
    }
}

} // namespace unity_plugins

using namespace unity_plugins;

///
/// @brief Callback to process a downloaded image. This is hooked up to the Texture Downloader
///
static void ZIPPY_INTERFACE_API ImageSaver_SaveImage(CameraImage cameraImage) { SaveImage(cameraImage); }

///
/// @brief Get the function pointer to the save image function
///
extern "C" ProcessImageFuncPtr ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API ImageSaver_GetSaveImageFuncPtr() { return ImageSaver_SaveImage; }

///
/// @brief set the save directory for the image captures
///
extern "C" void ZIPPY_INTERFACE_EXPORT ZIPPY_INTERFACE_API ImageSaver_SetSaveLocation(const char* location) { SetSaveDirectory(location); }
