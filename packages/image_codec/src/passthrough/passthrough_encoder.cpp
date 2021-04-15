#include "packages/image_codec/include/passthrough/passthrough_encoder.h"

namespace image_codec {

bool PassthroughEncoder::encode(const hal::Image& uncompressedImage, hal::Image& compressedImage) {
    compressedImage.CopyFrom(uncompressedImage);
    return true;
}
}
