#include "packages/image_codec/include/passthrough/passthrough_decoder.h"
#include "packages/image_codec/include/passthrough/passthrough_encoder.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace image_codec;

namespace {

static void OcvMatToCameraSample(const cv::Mat& image, hal::CameraSample& cameraSample) {

    if (image.type() != CV_8UC1 && image.type() != CV_8UC3) {
        LOG(ERROR) << "Passthrough Test: Unsupported image format";
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
        LOG(ERROR) << "Passthrough Test: Unsupported image format";
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

TEST(PassthroughCodecTest, EncodeAndDecodeGray) {
    PassthroughDecoder passthroughDecoder;
    PassthroughEncoder passthroughEncoder;

    hal::CameraSample inputImage;
    cv::Mat ocvInputImage = cv::imread("packages/image_codec/test/test.png", 0);
    OcvMatToCameraSample(ocvInputImage, inputImage);

    hal::CameraSample compressedImage, uncompressedImage;
    for (int i = 0; i < 5; i++) {
        passthroughEncoder.encode(inputImage.image(), *(compressedImage.mutable_image()));
        passthroughDecoder.decode(compressedImage.image(), *(uncompressedImage.mutable_image()));
    }

    EXPECT_EQ(inputImage.image().rows(), uncompressedImage.image().rows());
    EXPECT_EQ(inputImage.image().cols(), uncompressedImage.image().cols());
    EXPECT_EQ(inputImage.image().stride(), uncompressedImage.image().stride());
    EXPECT_EQ(inputImage.image().type(), uncompressedImage.image().type());
    EXPECT_EQ(inputImage.image().format(), uncompressedImage.image().format());
    EXPECT_EQ(inputImage.image().info().DebugString(), uncompressedImage.image().info().DebugString());
    EXPECT_EQ(inputImage.image().data(), compressedImage.image().data());
    EXPECT_EQ(inputImage.image().data(), uncompressedImage.image().data());
}

TEST(PassthroughCodecTest, EncodeAndDecodeColor) {
    PassthroughDecoder passthroughDecoder;
    PassthroughEncoder passthroughEncoder;

    hal::CameraSample inputImage;
    cv::Mat ocvInputImage = cv::imread("packages/image_codec/test/test.png");
    OcvMatToCameraSample(ocvInputImage, inputImage);

    hal::CameraSample compressedImage, uncompressedImage;
    for (int i = 0; i < 5; i++) {
        passthroughEncoder.encode(inputImage.image(), *(compressedImage.mutable_image()));
        passthroughDecoder.decode(compressedImage.image(), *(uncompressedImage.mutable_image()));
    }

    EXPECT_EQ(inputImage.image().rows(), uncompressedImage.image().rows());
    EXPECT_EQ(inputImage.image().cols(), uncompressedImage.image().cols());
    EXPECT_EQ(inputImage.image().stride(), uncompressedImage.image().stride());
    EXPECT_EQ(inputImage.image().type(), uncompressedImage.image().type());
    EXPECT_EQ(inputImage.image().format(), uncompressedImage.image().format());
    EXPECT_EQ(inputImage.image().info().DebugString(), uncompressedImage.image().info().DebugString());
    EXPECT_EQ(inputImage.image().data(), compressedImage.image().data());
    EXPECT_EQ(inputImage.image().data(), uncompressedImage.image().data());
}
