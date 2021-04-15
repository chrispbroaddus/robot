#pragma once

#include "packages/hal/proto/camera_sample.pb.h"

namespace image_codec {

///
/// Generic interface for image encoders
///
class ImageEncoder {
public:
    ImageEncoder() = default;
    ~ImageEncoder() = default;

    /// Takes a hal::Image as input and outputs a hal::Image with image data encoded using the codec being implemented
    virtual bool encode(const hal::Image& uncompressedImage, hal::Image& compressedImage) = 0;
};
}
