#include <cstddef>
#include <cstdint>
#include <vector>

#include "packages/core/include/image_view.h"
#include "packages/core/include/pixel_layout.h"

namespace image_codec {

// Encode a JPEG image and store the result in OUT.
bool EncodeJPEG(const core::ImageView<core::ImageType::uint8>& image, int quality, std::vector<uint8_t>* out);

// Encode a JPEG image and store the result in OUT.
bool EncodeJPEG(const core::ImageView<core::ImageType::rgb8>& image, int quality, std::vector<uint8_t>* out);

// Encode a JPEG image and store the result in OUT.
bool EncodeJPEG(const core::ImageView<core::ImageType::rgba8>& image, int quality, std::vector<uint8_t>* out);

// Encode a JPEG image and store the result in OUT.
bool EncodeJPEG(
    const uint8_t* data, size_t width, size_t height, size_t stride, core::ImageType layout, int quality, std::vector<uint8_t>* out);

} // namespace image_codec
