#pragma once

#include "packages/image_codec/include/image_encoder_interface.h"

#include "jpeglib.h"
#include <vector>

namespace image_codec {

struct JpegEncoderErrorMgr;

///
/// JpegEncoder class that encodes Images into jpeg format
/// NOT THREAD SAFE
///
class JpegEncoder : public ImageEncoder {
public:
    ///
    /// Creates a JpegEncoder object that can create compress Images by encoding the image data in jpeg format
    /// \param quality : (0 - 100) Non-linear parameter that determines the quality of the compressed image
    ///
    JpegEncoder(int quality);
    ~JpegEncoder();

    /// Takes a hal::Image as input and outputs a hal::Image with image data encoded in jpeg format
    bool encode(const hal::Image& uncompressedImage, hal::Image& compressedImage);

private:
    bool setupEncoder(const hal::Image& image);

    int m_quality;

    struct jpeg_compress_struct m_compressionInfo;
    std::unique_ptr<JpegEncoderErrorMgr> m_jpegErrorMgr;
    std::vector<uint8_t> m_outputBuffer;
};
}
