
#include "packages/core/include/dynamic_image.h"
#include "packages/core/include/aligned_memory_allocator.h"
#include "packages/core/include/dynamic_storage.h"
#include "packages/core/include/unmanaged_storage.h"
#include "gtest/gtest.h"

#include <vector>

using namespace core;

typedef DynamicStorage<uint8_scalar_t, AlignedMemoryAllocator> uint8_aligned_storage_t;
typedef DynamicStorage<rgb8_scalar_t, AlignedMemoryAllocator> rgb8_aligned_storage_t;

typedef UnmanagedStorage<uint8_pixel_t> uint8_unmanaged_storage_t;
typedef UnmanagedStorage<rgb8_pixel_t> rgb8_unmanaged_storage_t;

TEST(DynamicImage, ConstructorUint8) {

    constexpr size_t rows = 320;
    constexpr size_t cols = 240;
    constexpr size_t stride = 325;

    core::DynamicImage<ImageType::uint8, uint8_aligned_storage_t> image(rows, cols, stride);

    EXPECT_EQ(rows, image.rows());
    EXPECT_EQ(cols, image.cols());
    EXPECT_EQ(stride, image.stride());
}

TEST(DynamicImage, EvaluateSetGetUint8) {

    static const size_t rows = 320;
    static const size_t cols = 240;

    core::DynamicImage<ImageType::uint8, uint8_aligned_storage_t> image(rows, cols);

    image.view().at(160, 120) = 50;

    EXPECT_EQ(50, image.view().at(160, 120));
}

TEST(DynamicImage, ConstructorRGB8) {

    static const size_t rows = 320;
    static const size_t cols = 240;
    static const size_t stride = 325 * sizeof(rgb8_pixel_t);

    core::DynamicImage<ImageType::rgb8, rgb8_aligned_storage_t> image(rows, cols, stride);

    EXPECT_EQ(rows, image.rows());
    EXPECT_EQ(cols, image.cols());
    EXPECT_EQ(stride, image.stride());
}

TEST(DynamicImage, EvaluateSetGetRGB8) {

    static const size_t rows = 320;
    static const size_t cols = 240;

    core::DynamicImage<ImageType::rgb8, rgb8_aligned_storage_t> image(rows, cols);

    image.view().at(160, 120).r = 50;
    image.view().at(160, 120).g = 100;
    image.view().at(160, 120).b = 150;

    EXPECT_EQ(50, image.view().at(160, 120).r);
    EXPECT_EQ(100, image.view().at(160, 120).g);
    EXPECT_EQ(150, image.view().at(160, 120).b);
}

TEST(DynamicImage, PreallocatedMemoryUint8) {

    static const size_t rows = 320;
    static const size_t cols = 240;
    static const size_t stride = 320;

    std::vector<unsigned char> allocatedMemory(rows * stride);
    core::DynamicImage<ImageType::uint8, uint8_unmanaged_storage_t> image(rows, cols, stride, allocatedMemory.data());

    image.view().at(160, 120) = 10;

    EXPECT_EQ(10, image.view().at(160, 120));
}
