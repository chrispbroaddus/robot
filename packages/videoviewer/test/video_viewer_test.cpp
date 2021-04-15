#include "packages/videoviewer/include/video_viewer.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_topic_pub.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

#include <limits>

using namespace hal;
using namespace video_viewer;

namespace {

class VideoViewerTest : public ::testing::Test {
protected:
    VideoViewerTest() {}

    void SetUp() {

        const int publisherHighWaterMark = 1;
        const int lingerPeriodInMilliseconds = 1000;

        std::string serverAddress = "tcp://*:5567";
        std::string topic = "camera";

        m_context.reset(new zmq::context_t(1));
        m_cameraSamplePublisher.reset(new net::ZMQProtobufPublisher<CameraSample>(
            m_context.operator*(), serverAddress, publisherHighWaterMark, lingerPeriodInMilliseconds));
        LOG(INFO) << "Setup complete";
    }

    void publishImage(hal::Format imageFormat, hal::Type imageType) {

        int imageWidth = 640;
        int imageHeight = 480;

        CameraSample cameraSample;
        cameraSample.set_id(1);

        // Device
        hal::Device* device = new Device();
        device->set_name("Test");
        device->set_serialnumber(1234);

        // Image
        hal::Image* image = new hal::Image();
        image->set_cols(imageWidth);
        image->set_rows(imageHeight);
        image->set_type(imageType);
        image->set_format(imageFormat);

        if ((imageFormat == PB_LUMINANCE || imageFormat == PB_RAW) && (imageType == PB_UNSIGNED_BYTE || imageType == PB_BYTE)) {
            std::vector<unsigned char> bytes(imageWidth * imageHeight);
            for (int r = 0; r < imageHeight; r++) {
                for (int c = 0; c < imageWidth; c++) {
                    bytes[r * imageWidth + c] = (unsigned char)(r % 255);
                }
            }
            image->set_data(bytes.data(), bytes.size());
            image->set_stride(imageWidth);
        } else if ((imageFormat == PB_RGB) && (imageType == PB_UNSIGNED_BYTE || imageType == PB_BYTE)) {
            int bytesPerPixel = 3;
            uint32_t imageStride = (uint32_t)bytesPerPixel * imageWidth;
            std::vector<unsigned char> bytes(imageWidth * imageHeight * bytesPerPixel);
            for (int r = 0; r < imageHeight; r++) {
                for (int c = 0; c < imageWidth; c++) {
                    if (r < imageHeight / 2 && c < imageWidth / 2) {
                        bytes[r * imageStride + c * bytesPerPixel] = (unsigned char)(r % 255);
                        bytes[r * imageStride + c * bytesPerPixel + 1] = (unsigned char)(r % 255);
                        bytes[r * imageStride + c * bytesPerPixel + 2] = (unsigned char)(r % 255);
                    } else if (r < imageHeight / 2 && c < imageWidth) {
                        bytes[r * imageStride + c * bytesPerPixel] = (unsigned char)(r % 255);
                        bytes[r * imageStride + c * bytesPerPixel + 1] = 0;
                        bytes[r * imageStride + c * bytesPerPixel + 2] = 0;
                    } else if (r < imageHeight && c < imageWidth / 2) {
                        bytes[r * imageStride + c * bytesPerPixel] = 0;
                        bytes[r * imageStride + c * bytesPerPixel + 1] = (unsigned char)((r - imageHeight / 2) % 255);
                        bytes[r * imageStride + c * bytesPerPixel + 2] = 0;
                    } else {
                        bytes[r * imageStride + c * bytesPerPixel] = 0;
                        bytes[r * imageStride + c * bytesPerPixel + 1] = 0;
                        bytes[r * imageStride + c * bytesPerPixel + 2] = (unsigned char)((r - imageHeight / 2) % 255);
                    }
                }
            }
            image->set_data(bytes.data(), bytes.size());
            image->set_stride(imageStride);
        } else if ((imageFormat == PB_LUMINANCE || imageFormat == PB_RANGE) && imageType == PB_FLOAT) {
            int bytesPerPixel = 4;
            uint32_t imageStride = (uint32_t)imageWidth;
            std::vector<float> data(imageWidth * imageHeight);
            for (int r = 0; r < imageHeight; r++) {
                for (int c = 0; c < imageWidth; c++) {
                    if (r < imageHeight / 2 && c < imageWidth / 2) {
                        data[r * imageWidth + c] = 2.0f * r / imageHeight;
                    } else if (r < imageHeight / 2 && c < imageWidth) {
                        if (c % 2 == 0)
                            data[r * imageWidth + c] = std::numeric_limits<float>::quiet_NaN();
                        else
                            data[r * imageWidth + c] = std::numeric_limits<float>::infinity();
                    } else if (r < imageHeight && c < imageWidth / 2) {
                        data[r * imageWidth + c] = -r * 4.0f;
                    } else {
                        data[r * imageWidth + c] = r * 5.0f;
                    }
                }
            }
            image->set_data(data.data(), data.size() * bytesPerPixel);
            image->set_stride(imageStride);
        } else {
            LOG(INFO) << "Unsupported image format or type";
        }

        // HardwareTimestamp
        core::HardwareTimestamp* hardwareTimestamp = new core::HardwareTimestamp();
        hardwareTimestamp->set_nanos(0);

        // SystemTimestamp
        core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
        systemTimestamp->set_nanos(0);

        // Set the CameraSample
        cameraSample.set_allocated_device(device);
        cameraSample.set_allocated_image(image);
        cameraSample.set_allocated_systemtimestamp(systemTimestamp);
        cameraSample.set_allocated_hardwaretimestamp(hardwareTimestamp);

        m_cameraSamplePublisher->send(cameraSample, "camera");
        LOG(INFO) << "Sent Image";
    }

    void TearDown() { LOG(INFO) << "Teardown complete"; }

    std::unique_ptr<zmq::context_t> m_context;
    std::unique_ptr<net::ZMQProtobufPublisher<CameraSample> > m_cameraSamplePublisher;
};

TEST_F(VideoViewerTest, canCreate) {

    std::string serverAddress = "tcp://localhost:5567";
    std::string topic = "camera";
    EXPECT_NO_THROW(video_viewer::VideoViewer videoViewer(serverAddress, topic));
}

TEST_F(VideoViewerTest, canDisplayMonochrome) {

    std::string serverAddress = "tcp://localhost:5567";
    std::string topic = "camera";
    video_viewer::VideoViewer videoViewer(serverAddress, topic);

    publishImage(PB_LUMINANCE, PB_UNSIGNED_BYTE);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(VideoViewerTest, canDisplayColor) {

    std::string serverAddress = "tcp://localhost:5567";
    std::string topic = "camera";
    video_viewer::VideoViewer videoViewer(serverAddress, topic);

    publishImage(PB_RGB, PB_UNSIGNED_BYTE);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

TEST_F(VideoViewerTest, canDisplayFloat) {

    std::string serverAddress = "tcp://localhost:5567";
    std::string topic = "camera";
    video_viewer::VideoViewer videoViewer(serverAddress, topic);

    publishImage(PB_LUMINANCE, PB_FLOAT);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}
}
