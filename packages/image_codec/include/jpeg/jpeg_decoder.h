#pragma once

#include "packages/image_codec/include/image_decoder_interface.h"

#include "jpeglib.h"
#include <vector>

namespace image_codec {

struct JpegDecoderErrorMgr;

///
/// JpegDecoder class that decodes Images stored in jpeg format
/// NOT THREAD SAFE
///
class JpegDecoder : public ImageDecoder {
public:
    ///
    /// Creates a JpegDecoder object that can decode Images stored in jpeg format
    ///
    JpegDecoder();
    ~JpegDecoder();

    /// Takes a hal::Image with image data encoded in jpeg format as input and outputs a decoded hal::Image
    bool decode(const hal::Image& compressedImage, hal::Image& uncompressedImage);

private:
    struct jpeg_decompress_struct m_compressionInfo;
    std::unique_ptr<JpegDecoderErrorMgr> m_jpegErrorMgr;
};
}
