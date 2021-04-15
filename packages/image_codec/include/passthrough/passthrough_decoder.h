#pragma once

#include "packages/image_codec/include/image_decoder_interface.h"

#include <vector>

namespace image_codec {

///
/// PassthroughDecoder class that outputs the input image as is without modifications
///
class PassthroughDecoder : public ImageDecoder {
public:
    ///
    /// Creates a PassthroughDecoder object that passes input images to output as is without modifications
    ///
    PassthroughDecoder() = default;
    ~PassthroughDecoder() = default;

    /// Passes the input hal::Image as is to the output without any modifications
    bool decode(const hal::Image& compressedImage, hal::Image& uncompressedImage);
};
}
