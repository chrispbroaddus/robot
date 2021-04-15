#include "packages/image_codec/include/passthrough/passthrough_decoder.h"

namespace image_codec {

bool PassthroughDecoder::decode(const hal::Image& compressedImage, hal::Image& uncompressedImage) {
    uncompressedImage.CopyFrom(compressedImage);
    return true;
}
}
