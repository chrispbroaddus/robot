#pragma once

#include "packages/image_codec/include/image_decoder_interface.h"

#include "png.h"
#include <vector>

namespace image_codec {

///
/// PngDecoder class that decodes Images stored in png format
///
class PngDecoder : public ImageDecoder {
public:
    ///
    /// Creates a PngDecoder object that decodes Images stored in png format
    ///
    PngDecoder();
    ~PngDecoder();

    /// Takes a hal::Image with image data encoded in png format as input and outputs a decoded hal::Image
    bool decode(const hal::Image& compressedImage, hal::Image& uncompressedImage);
};
}
