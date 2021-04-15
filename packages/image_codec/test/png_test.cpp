#include <stdlib.h>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/core/include/aligned_memory_allocator.h"
#include "packages/core/include/dynamic_image.h"
#include "packages/core/include/dynamic_storage.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/image_codec/include/png.h"
#include "packages/image_codec/include/png/png_decoder.h"
#include "packages/image_codec/include/png/png_encoder.h"

#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

typedef core::DynamicStorage<core::uint8_scalar_t, core::AlignedMemoryAllocator> uint8_aligned_storage_t;
typedef core::DynamicStorage<core::rgb8_scalar_t, core::AlignedMemoryAllocator> rgb8_aligned_storage_t;

using namespace image_codec;

namespace {

void OcvMatToCameraSample(const cv::Mat& image, hal::CameraSample& cameraSample) {

    if (image.type() != CV_8UC1 && image.type() != CV_8UC3) {
        LOG(ERROR) << "Png Test: Unsupported image format";
        CHECK(false);
    }

    int bytes_per_pixel;

    // Device
    hal::Device* device = new hal::Device();
    CHECK_NOTNULL(device);
    device->set_name("depth");
    device->set_serialnumber(0);

    // Image
    hal::Image* halImage = new hal::Image();
    CHECK_NOTNULL(halImage);
    halImage->set_cols(static_cast<uint32_t>(image.cols));
    halImage->set_rows(static_cast<uint32_t>(image.rows));
    if (image.type() == CV_8UC1) {
        halImage->set_type(hal::PB_UNSIGNED_BYTE); // PB_DATA_TYPE
        halImage->set_format(hal::PB_LUMINANCE); // PB_RANGE
        halImage->set_stride(static_cast<uint32_t>(image.cols));
        bytes_per_pixel = 1;
    } else if (image.type() == CV_8UC3) {
        halImage->set_type(hal::PB_UNSIGNED_BYTE); // PB_DATA_TYPE
        halImage->set_format(hal::PB_RGB); // PB_RANGE
        bytes_per_pixel = 3;
        halImage->set_stride(static_cast<uint32_t>(image.cols * bytes_per_pixel));
    } else {
        LOG(ERROR) << "Png Test: Unsupported image format";
        CHECK(false);
    }

    halImage->set_data(image.data, static_cast<uint32_t>(image.rows * image.cols * bytes_per_pixel));

    // Timestamps
    core::HardwareTimestamp* hardwareTimestamp = new core::HardwareTimestamp();
    CHECK_NOTNULL(hardwareTimestamp);
    core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
    CHECK_NOTNULL(systemTimestamp);

    // Set the CameraSample
    cameraSample.set_allocated_device(device);
    cameraSample.set_allocated_image(halImage);
    cameraSample.set_allocated_systemtimestamp(systemTimestamp);
    cameraSample.set_allocated_hardwaretimestamp(hardwareTimestamp);
    cameraSample.set_id(0);
}
}

TEST(PngTest, EncodeGray) {
    // generate a synthetic image
    core::DynamicImage<core::ImageType::uint8, uint8_aligned_storage_t> image(20, 30);
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 30; j++) {
            image.view().at(i, j) = i + j;
        }
    }

    std::vector<uint8_t> buf;
    EXPECT_TRUE(EncodePNG(image.view(), 4, &buf));

    for (auto x : buf) {
        printf("0x%02x, ", x);
    }
    printf("\n");

    std::vector<uint8_t> expected{ 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00,
        0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x14, 0x08, 0x00, 0x00, 0x00, 0x00, 0xbf, 0xc0, 0xd2, 0x18, 0x00, 0x00, 0x00, 0x18, 0x49, 0x44,
        0x41, 0x54, 0x28, 0x53, 0x63, 0x64, 0x60, 0xa4, 0x00, 0x30, 0xa1, 0x0b, 0xa0, 0x82, 0x51, 0x69, 0xac, 0x60, 0xa4, 0x4a, 0x03, 0x00,
        0x04, 0xe1, 0x02, 0x7e, 0x32, 0x5d, 0xef, 0x88, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82 };

    EXPECT_EQ(buf.size(), expected.size());
    for (size_t i = 0; i < buf.size(); i++) {
        EXPECT_EQ(buf[i], expected[i]) << "at " << i;
    }
}

TEST(PngTest, EncodeAndDecodeGray) {
    PngDecoder pngDecoder;
    PngEncoder pngEncoder(4);

    hal::CameraSample inputImage;
    cv::Mat ocvImage = cv::imread("packages/image_codec/test/test.png", 0);
    OcvMatToCameraSample(ocvImage, inputImage);

    hal::CameraSample compressedImage, uncompressedImage;
    for (int i = 0; i < 5; i++) {
        pngEncoder.encode(inputImage.image(), *(compressedImage.mutable_image()));
        pngDecoder.decode(compressedImage.image(), *(uncompressedImage.mutable_image()));
    }

    EXPECT_LT(compressedImage.image().ByteSize(), inputImage.image().ByteSize());
    EXPECT_EQ(inputImage.image().rows(), uncompressedImage.image().rows());
    EXPECT_EQ(inputImage.image().cols(), uncompressedImage.image().cols());
    EXPECT_EQ(inputImage.image().stride(), uncompressedImage.image().stride());
    EXPECT_EQ(inputImage.image().type(), uncompressedImage.image().type());
    EXPECT_EQ(inputImage.image().format(), uncompressedImage.image().format());
    EXPECT_EQ(inputImage.image().data(), uncompressedImage.image().data());
    EXPECT_EQ(inputImage.image().info().DebugString(), uncompressedImage.image().info().DebugString());
}

TEST(PngTest, EncodeAndDecodeColor) {
    PngDecoder pngDecoder;
    PngEncoder pngEncoder(4);

    hal::CameraSample inputImage;
    cv::Mat ocvImage = cv::imread("packages/image_codec/test/test.png");
    OcvMatToCameraSample(ocvImage, inputImage);

    hal::CameraSample compressedImage, uncompressedImage;
    for (int i = 0; i < 5; i++) {
        pngEncoder.encode(inputImage.image(), *(compressedImage.mutable_image()));
        pngDecoder.decode(compressedImage.image(), *(uncompressedImage.mutable_image()));
    }

    EXPECT_LT(compressedImage.image().ByteSize(), inputImage.image().ByteSize());
    EXPECT_EQ(inputImage.image().rows(), uncompressedImage.image().rows());
    EXPECT_EQ(inputImage.image().cols(), uncompressedImage.image().cols());
    EXPECT_EQ(inputImage.image().stride(), uncompressedImage.image().stride());
    EXPECT_EQ(inputImage.image().type(), uncompressedImage.image().type());
    EXPECT_EQ(inputImage.image().format(), uncompressedImage.image().format());
    EXPECT_EQ(inputImage.image().data(), uncompressedImage.image().data());
    EXPECT_EQ(inputImage.image().info().DebugString(), uncompressedImage.image().info().DebugString());
}
