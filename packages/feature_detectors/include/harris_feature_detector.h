
#pragma once

#include "packages/core/include/aligned_memory_allocator.h"
#include "packages/core/include/dynamic_image.h"
#include "packages/core/include/dynamic_storage.h"
#include "packages/core/include/image_view.h"
#include "packages/core/include/unmanaged_storage.h"
#include "packages/feature_detectors/proto/feature_point.pb.h"

namespace feature_detectors {

/// Find the local maxima of the harris strength image with a 5x5 window. The pixels within 3 pixels around
/// the image are ignored since these represent the non-valid pixels from the binomial harris algorithm.
void nonMaxSuppression5x5(std::vector<FeaturePoint>& points, const core::ImageView<core::ImageType::float32>& image);

/// @brief Computes the harris strength image with one pass of the image. The derivatives and convolution are computed
/// in-place down the image and no memory is allocated on the heap.
/// \param strengths Destination image with same size as source
/// \param Ixx Temporary image of size 5*sourceWidth
/// \param Iyy Temporary image of size 5*sourceWidth
/// \param Ixy Temporary image of size 5*sourceWidth
/// \param Gxx Temporary image of size 5*sourceWidth
/// \param Gyy Temporary image of size 5*sourceWidth
/// \param Gxy Temporary image of size 5*sourceWidth
/// \param image Source image
/// \param k Harris coefficient
void computeHarrisStrengths(core::ImageView<core::ImageType::float32>& strengths, core::ImageView<core::ImageType::int16>& Ixx,
    core::ImageView<core::ImageType::int16>& Iyy, core::ImageView<core::ImageType::int16>& Ixy,
    core::ImageView<core::ImageType::int32>& Gxx, core::ImageView<core::ImageType::int32>& Gyy,
    core::ImageView<core::ImageType::int32>& Gxy, const core::ImageView<core::ImageType::uint8>& image, const float k);

/// @brief Harris features is a corner detector based on the differential of an image.
class HarrisFeatureDetector {
public:
    typedef core::DynamicStorage<core::uint8_scalar_t, core::AlignedMemoryAllocator> uint8_aligned_storage_t;
    typedef core::DynamicStorage<core::int16_scalar_t, core::AlignedMemoryAllocator> int16_aligned_storage_t;
    typedef core::DynamicStorage<core::int32_scalar_t, core::AlignedMemoryAllocator> int32_aligned_storage_t;
    typedef core::DynamicStorage<core::float32_scalar_t, core::AlignedMemoryAllocator> float32_aligned_storage_t;

    HarrisFeatureDetector(const float k)
        : m_k(k) {}

    ~HarrisFeatureDetector() = default;

    /// Detect the harris features in an image.
    /// @return vector of feature points
    const std::vector<FeaturePoint>& detect(const core::ImageView<core::ImageType::uint8>& image);

    /// @return strength image
    const core::DynamicImage<core::ImageType::float32, float32_aligned_storage_t>& strengths() const { return m_strengths; };

private:
    const float m_k;
    core::DynamicImage<core::ImageType::int16, int16_aligned_storage_t> m_Ixx;
    core::DynamicImage<core::ImageType::int16, int16_aligned_storage_t> m_Iyy;
    core::DynamicImage<core::ImageType::int16, int16_aligned_storage_t> m_Ixy;
    core::DynamicImage<core::ImageType::int32, int32_aligned_storage_t> m_Gxx;
    core::DynamicImage<core::ImageType::int32, int32_aligned_storage_t> m_Gyy;
    core::DynamicImage<core::ImageType::int32, int32_aligned_storage_t> m_Gxy;
    core::DynamicImage<core::ImageType::float32, float32_aligned_storage_t> m_strengths;
    std::vector<FeaturePoint> m_points;
};
}