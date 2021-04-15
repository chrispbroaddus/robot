#pragma once

#include "packages/image_codec/include/image_encoder_interface.h"

#include "png.h"
#include <vector>

namespace image_codec {

///
/// PngEncoder class that encodes Images into png format
///
class PngEncoder : public ImageEncoder {
public:
    ///
    /// Creates a PngEncoder object that can create compress Images by encoding the image data in png format
    /// \param compressionLevel : (0 - 9) parameter that determines the how much space is saved after compression
    ///
    PngEncoder(int compressionLevel);
    ~PngEncoder();

    /// Takes a hal::Image as input and outputs a hal::Image with image data encoded in png format
    bool encode(const hal::Image& uncompressedImage, hal::Image& compressedImage);

private:
    int m_compressionLevel;
};
}
