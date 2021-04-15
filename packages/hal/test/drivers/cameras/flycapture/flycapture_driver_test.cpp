#include "packages/hal/include/drivers/cameras/flycapture/flycapture_driver.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

#include <thread>

using namespace hal;
using namespace FlyCapture2;

class HalFlycaptureDriverTest : public testing::Test {
protected:
    HalFlycaptureDriverTest() {}

    void SetUp() {
        FlyCapture2::Error error;
        FlyCapture2::BusManager busManager;
        unsigned int numCameras = 0;

        error = busManager.GetNumOfCameras(&numCameras);
        if (error != FlyCapture2::PGRERROR_OK) {
            LOG(ERROR) << "Cannot get number of connected cameras";
            throw std::runtime_error("Cannot get number of connected cameras");
        }

        if (numCameras == 0) {
            LOG(ERROR) << "No cameras connected";
            throw std::runtime_error("No cameras connected");
        }
        LOG(INFO) << "Number of cameras detected: " << numCameras;

        error = busManager.GetCameraSerialNumberFromIndex(0, &testSerialNumber);
        if (error != FlyCapture2::PGRERROR_OK) {
            LOG(ERROR) << "Cannot get serial number of connected camera";
            throw std::runtime_error("Cannot get serial number of connected camera");
        }
    }

    void TearDown() { testSerialNumber = 0; }

    uint32_t testSerialNumber = 0;
};

TEST_F(HalFlycaptureDriverTest, canInstantiate) {

    uint64_t serialNumber = testSerialNumber;
    GrabMode grabMode = DROP_FRAMES;
    uint32_t captureTimeoutInMilliseconds = 10;
    Mode format7Mode = MODE_0;
    PixelFormat pixelFormat = PIXEL_FORMAT_MONO8;
    uint32_t width = 2448;
    uint32_t height = 2048;
    uint32_t left = 0;
    uint32_t top = 0;
    uint32_t numBuffers = 10;
    float fps = 30;
    bool externalTrigger = false;
    uint32_t AERoILeft = 0;
    uint32_t AERoITop = 0;
    uint32_t AERoIWidth = 2448;
    uint32_t AERoIHeight = 2048;

    EXPECT_NO_THROW(FlycaptureDriver flycaptureDriver(serialNumber, grabMode, format7Mode, pixelFormat, left, top, width, height, fps,
        numBuffers, captureTimeoutInMilliseconds, externalTrigger, AERoILeft, AERoITop, AERoIWidth, AERoIHeight));
}

TEST_F(HalFlycaptureDriverTest, canCaptureImages) {

    uint64_t serialNumber = testSerialNumber;
    GrabMode grabMode = DROP_FRAMES;
    uint32_t captureTimeoutInMilliseconds = 10;
    Mode format7Mode = MODE_0;
    PixelFormat pixelFormat = PIXEL_FORMAT_RAW8;
    uint32_t width = 2448;
    uint32_t height = 2048;
    uint32_t left = 0;
    uint32_t top = 0;
    uint32_t numBuffers = 10;
    float fps = 30;
    bool externalTrigger = false;
    uint32_t AERoILeft = 0;
    uint32_t AERoITop = 0;
    uint32_t AERoIWidth = 2448;
    uint32_t AERoIHeight = 2048;

    FlycaptureDriver flycaptureDriver(serialNumber, grabMode, format7Mode, pixelFormat, left, top, width, height, fps, numBuffers,
        captureTimeoutInMilliseconds, externalTrigger, AERoILeft, AERoITop, AERoIWidth, AERoIHeight);

    CameraSample cameraSample;
    int counter = 0;
    while (!flycaptureDriver.capture(cameraSample) && counter < 10) {
        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    EXPECT_EQ(width, cameraSample.image().cols());
    EXPECT_EQ(height, cameraSample.image().rows());

    FILE* imagefile;
    imagefile = fopen("image.pgm", "wb");
    if (imagefile == NULL) {
        perror("Can't create output file");
    }
    fprintf(imagefile, "P5\n%u %u\n255\n", width, height);
    const unsigned char* imagePtr = (const unsigned char*)(cameraSample.image().data().data());
    fwrite(imagePtr, 1, height * width * 1, imagefile);
    fclose(imagefile);
}

TEST_F(HalFlycaptureDriverTest, canCaptureImagesWithTrigger) {

    uint64_t serialNumber = testSerialNumber;
    GrabMode grabMode = DROP_FRAMES;
    uint32_t captureTimeoutInMilliseconds = 100;
    Mode format7Mode = MODE_0;
    PixelFormat pixelFormat = PIXEL_FORMAT_MONO8;
    uint32_t width = 2448;
    uint32_t height = 2048;
    uint32_t left = 0;
    uint32_t top = 0;
    uint32_t numBuffers = 10;
    float fps = 30;
    bool externalTrigger = true;
    uint32_t AERoILeft = 0;
    uint32_t AERoITop = 0;
    uint32_t AERoIWidth = 2448;
    uint32_t AERoIHeight = 2048;

    FlycaptureDriver flycaptureDriver(serialNumber, grabMode, format7Mode, pixelFormat, left, top, width, height, fps, numBuffers,
        captureTimeoutInMilliseconds, externalTrigger, AERoILeft, AERoITop, AERoIWidth, AERoIHeight);

    CameraSample cameraSample;
    int counter = 0;
    while (!flycaptureDriver.capture(cameraSample) && counter < 10) {
        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    EXPECT_EQ(width, cameraSample.image().cols());
    EXPECT_EQ(height, cameraSample.image().rows());

    FILE* imagefile;
    imagefile = fopen("image.pgm", "wb");
    if (imagefile == NULL) {
        perror("Can't create output file");
    }
    fprintf(imagefile, "P5\n%u %u\n255\n", width, height);
    const unsigned char* imagePtr = (const unsigned char*)(cameraSample.image().data().data());
    fwrite(imagePtr, 1, height * width * 1, imagefile);
    fclose(imagefile);
}
