#include <cstddef>
#include <cstdint>
#include <vector>

#include "packages/core/include/image_view.h"
#include "packages/core/include/pixel_layout.h"

namespace image_codec {

// Encode a PNG image and store the result in OUT.
bool EncodePNG(const core::ImageView<core::ImageType::uint8>& image, int compression, std::vector<uint8_t>* out);

// Encode a PNG image and store the result in OUT.
bool EncodePNG(const core::ImageView<core::ImageType::rgb8>& image, int compression, std::vector<uint8_t>* out);

// Encode a monochrome image to PNG and return the result in the provided vector.
bool EncodePNG(
    const uint8_t* data, size_t width, size_t height, size_t stride, core::ImageType layout, int compression, std::vector<uint8_t>* out);

} // namespace image_codec
