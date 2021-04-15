#include <stdlib.h>

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "packages/core/include/aligned_memory_allocator.h"
#include "packages/core/include/dynamic_image.h"
#include "packages/core/include/dynamic_storage.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/image_codec/include/jpeg.h"
#include "packages/image_codec/include/jpeg/jpeg_decoder.h"
#include "packages/image_codec/include/jpeg/jpeg_encoder.h"

#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

typedef core::DynamicStorage<core::uint8_scalar_t, core::AlignedMemoryAllocator> uint8_aligned_storage_t;
typedef core::DynamicStorage<core::rgb8_scalar_t, core::AlignedMemoryAllocator> rgb8_aligned_storage_t;

using namespace image_codec;

namespace {

static void OcvMatToCameraSample(const cv::Mat& image, hal::CameraSample& cameraSample) {

    if (image.type() != CV_8UC1 && image.type() != CV_8UC3) {
        LOG(ERROR) << "Jpeg Test: Unsupported image format";
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
        LOG(ERROR) << "Jpeg Test: Unsupported image format";
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

static void cameraSampleToOcvMat(const hal::CameraSample& cameraSample, cv::Mat& image) {

    int rows = cameraSample.image().rows();
    int cols = cameraSample.image().cols();
    int stride = cameraSample.image().stride();
    unsigned char* imageData = (unsigned char*)(cameraSample.image().data().data());

    if (cameraSample.image().type() == hal::PB_UNSIGNED_BYTE || cameraSample.image().type() == hal::PB_BYTE) {
        if (cameraSample.image().format() == hal::PB_LUMINANCE || cameraSample.image().format() == hal::PB_RAW) {
            image = cv::Mat(rows, cols, CV_8UC1, imageData, stride);
        } else if (cameraSample.image().format() == hal::PB_RGB) {
            image = cv::Mat(rows, cols, CV_8UC3, imageData, stride);
        } else if (cameraSample.image().format() == hal::PB_RGBA) {
            image = cv::Mat(rows, cols, CV_8UC4, imageData, stride);
        } else {
            LOG(ERROR) << "Jpeg Test: Unsupported image format";
            CHECK(false);
        }
    } else {
        LOG(ERROR) << "Jpeg Test: Unsupported image type";
        CHECK(false);
    }
}
}

TEST(JPEGTest, EncodeGray) {
    // generate a synthetic image
    core::DynamicImage<core::ImageType::uint8, uint8_aligned_storage_t> image(20, 30);
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 30; j++) {
            image.view().at(i, j) = uint8_t(i + j);
        }
    }

    std::vector<uint8_t> buf;
    EXPECT_TRUE(EncodeJPEG(image.view(), 4, &buf));

    std::vector<uint8_t> expected{ 0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00,
        0x01, 0x00, 0x00, 0xff, 0xdb, 0x00, 0x43, 0x00, 0xc8, 0x8a, 0x96, 0xaf, 0x96, 0x7d, 0xc8, 0xaf, 0xa3, 0xaf, 0xe1, 0xd5, 0xc8, 0xee,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x0b, 0x08, 0x00, 0x14, 0x00, 0x1e, 0x01, 0x01, 0x11, 0x00, 0xff, 0xc4, 0x00,
        0x1f, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0xff, 0xc4, 0x00, 0xb5, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05,
        0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61,
        0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82,
        0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44,
        0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,
        0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
        0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2,
        0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5,
        0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xff, 0xda, 0x00, 0x08, 0x01, 0x01, 0x00,
        0x00, 0x3f, 0x00, 0x8a, 0x8a, 0x5a, 0x29, 0x29, 0x68, 0xa2, 0x8a, 0x28, 0xa5, 0xaf, 0xff, 0xd9 };

    ASSERT_EQ(expected.size(), buf.size());
    for (size_t i = 0; i < buf.size(); i++) {
        EXPECT_EQ(expected[i], buf[i]) << "i=" << i;
    }
}

TEST(JpegTest, EncodeAndDecodeGray) {
    JpegDecoder jpegDecoder;
    JpegEncoder jpegEncoder(80);

    hal::CameraSample inputImage;
    cv::Mat ocvInputImage = cv::imread("packages/image_codec/test/test.png", 0);
    OcvMatToCameraSample(ocvInputImage, inputImage);

    hal::CameraSample compressedImage, uncompressedImage;
    for (int i = 0; i < 5; i++) {
        jpegEncoder.encode(inputImage.image(), *(compressedImage.mutable_image()));
        jpegDecoder.decode(compressedImage.image(), *(uncompressedImage.mutable_image()));
    }

    EXPECT_LT(compressedImage.image().ByteSize(), inputImage.image().ByteSize());
    EXPECT_EQ(inputImage.image().rows(), uncompressedImage.image().rows());
    EXPECT_EQ(inputImage.image().cols(), uncompressedImage.image().cols());
    EXPECT_EQ(inputImage.image().stride(), uncompressedImage.image().stride());
    EXPECT_EQ(inputImage.image().type(), uncompressedImage.image().type());
    EXPECT_EQ(inputImage.image().format(), uncompressedImage.image().format());
    EXPECT_EQ(inputImage.image().info().DebugString(), uncompressedImage.image().info().DebugString());

    cv::Scalar mean;
    cv::Mat ocvOutputImage;
    cameraSampleToOcvMat(uncompressedImage, ocvOutputImage);
    mean = cv::mean(ocvInputImage - ocvOutputImage);
    EXPECT_LE(mean.val[0], 5);
}

TEST(JpegTest, EncodeAndDecodeColor) {
    JpegDecoder jpegDecoder;
    JpegEncoder jpegEncoder(80);

    hal::CameraSample inputImage;
    cv::Mat ocvInputImage = cv::imread("packages/image_codec/test/test.png");
    OcvMatToCameraSample(ocvInputImage, inputImage);

    hal::CameraSample compressedImage, uncompressedImage;
    for (int i = 0; i < 5; i++) {
        jpegEncoder.encode(inputImage.image(), *(compressedImage.mutable_image()));
        jpegDecoder.decode(compressedImage.image(), *(uncompressedImage.mutable_image()));
    }

    EXPECT_LT(compressedImage.image().ByteSize(), inputImage.image().ByteSize());
    EXPECT_EQ(inputImage.image().rows(), uncompressedImage.image().rows());
    EXPECT_EQ(inputImage.image().cols(), uncompressedImage.image().cols());
    EXPECT_EQ(inputImage.image().stride(), uncompressedImage.image().stride());
    EXPECT_EQ(inputImage.image().type(), uncompressedImage.image().type());
    EXPECT_EQ(inputImage.image().format(), uncompressedImage.image().format());
    EXPECT_EQ(inputImage.image().info().DebugString(), uncompressedImage.image().info().DebugString());

    cv::Scalar mean;
    cv::Mat ocvOutputImage;
    cameraSampleToOcvMat(uncompressedImage, ocvOutputImage);
    mean = cv::mean(ocvInputImage - ocvOutputImage);
    EXPECT_LE(mean.val[0], 5);
    EXPECT_LE(mean.val[1], 5);
    EXPECT_LE(mean.val[2], 5);
}
