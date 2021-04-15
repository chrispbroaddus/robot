#pragma once

#include "packages/hal/proto/camera_sample.pb.h"

namespace image_codec {

///
/// Generic interface for image decoders
///
class ImageDecoder {
public:
    ImageDecoder() = default;
    ~ImageDecoder() = default;

    /// Takes a hal::Image with encoded image data as input and outputs a decoded hal::Image using the codec being implemented
    virtual bool decode(const hal::Image& compressedImage, hal::Image& uncompressedImage) = 0;
};
}
