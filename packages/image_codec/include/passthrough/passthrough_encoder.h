#pragma once

#include "packages/image_codec/include/image_encoder_interface.h"

#include <vector>

namespace image_codec {

///
/// PassthroughEncoder class that outputs the input image as is without modifications
///
class PassthroughEncoder : public ImageEncoder {
public:
    ///
    /// Creates a PassthroughEncoder object that passes input images to output as is without modifications
    ///
    PassthroughEncoder() = default;
    ~PassthroughEncoder() = default;

    /// Passes the input hal::Image as is to the output without any modifications
    bool encode(const hal::Image& uncompressedImage, hal::Image& compressedImage);
};
}
