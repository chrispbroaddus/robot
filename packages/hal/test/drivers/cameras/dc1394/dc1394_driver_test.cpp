#include "packages/hal/include/drivers/cameras/dc1394/dc1394_driver.h"
#include "gtest/gtest.h"
#include <dc1394/dc1394.h>

#include <thread>

using namespace hal;

class HalDc1394DriverTest : public testing::Test {
protected:
    HalDc1394DriverTest() {}

    void SetUp() {
        dc1394_t* bus;
        dc1394error_t err;
        dc1394camera_list_t* list;

        bus = dc1394_new();
        if (!bus) {
            throw std::runtime_error("Dc1394: Bus creation failed");
        }
        err = dc1394_camera_enumerate(bus, &list);
        if (err != DC1394_SUCCESS) {
            throw std::runtime_error("DC1394: Camera enumeration failed");
        }
        if (list->num == 0) {
            throw std::runtime_error("No cameras found");
        }
        testGuid = list->ids[0].guid;
    }

    void TearDown() { testGuid = 0; }

    uint64_t testGuid = 0;
};

TEST_F(HalDc1394DriverTest, badLeftRoi) {
    uint64_t guid = testGuid;
    dc1394video_mode_t videoMode = DC1394_VIDEO_MODE_FORMAT7_0;
    dc1394color_coding_t colorCoding = DC1394_COLOR_CODING_MONO8;
    uint32_t left = 2441;
    uint32_t top = 0;
    uint32_t imageWidth = 2440;
    uint32_t imageHeight = 2046;
    float fps = 60;
    uint32_t nDma = 4;
    bool externalTrigger = false;

    EXPECT_THROW(Dc1394Driver driver(guid, videoMode, colorCoding, left, top, imageWidth, imageHeight, fps, nDma, externalTrigger),
        std::runtime_error);
}

TEST_F(HalDc1394DriverTest, badTopRoi) {
    uint64_t guid = testGuid;
    dc1394video_mode_t videoMode = DC1394_VIDEO_MODE_FORMAT7_0;
    dc1394color_coding_t colorCoding = DC1394_COLOR_CODING_MONO8;
    uint32_t left = 0;
    uint32_t top = 2447;
    uint32_t imageWidth = 2440;
    uint32_t imageHeight = 2046;
    float fps = 60;
    uint32_t nDma = 4;
    bool externalTrigger = false;

    EXPECT_THROW(Dc1394Driver driver(guid, videoMode, colorCoding, left, top, imageWidth, imageHeight, fps, nDma, externalTrigger),
        std::runtime_error);
}

TEST_F(HalDc1394DriverTest, badNDma) {
    uint64_t guid = testGuid;
    dc1394video_mode_t videoMode = DC1394_VIDEO_MODE_FORMAT7_0;
    dc1394color_coding_t colorCoding = DC1394_COLOR_CODING_MONO8;
    uint32_t left = 0;
    uint32_t top = 0;
    uint32_t imageWidth = 2440;
    uint32_t imageHeight = 2046;
    float fps = 60;
    uint32_t nDma = 0;
    bool externalTrigger = false;

    EXPECT_THROW(Dc1394Driver driver(guid, videoMode, colorCoding, left, top, imageWidth, imageHeight, fps, nDma, externalTrigger),
        std::runtime_error);
}

TEST_F(HalDc1394DriverTest, canInstantiate) {
    uint64_t guid = testGuid;
    dc1394video_mode_t videoMode = DC1394_VIDEO_MODE_FORMAT7_0;
    dc1394color_coding_t colorCoding = DC1394_COLOR_CODING_MONO8;
    uint32_t left = 0;
    uint32_t top = 0;
    uint32_t imageWidth = 2440;
    uint32_t imageHeight = 2046;
    float fps = 60;
    uint32_t nDma = 4;
    bool externalTrigger = false;

    EXPECT_NO_THROW(Dc1394Driver driver(guid, videoMode, colorCoding, left, top, imageWidth, imageHeight, fps, nDma, externalTrigger));
}

TEST_F(HalDc1394DriverTest, canCaptureImages) {
    uint64_t guid = testGuid;
    dc1394video_mode_t videoMode = DC1394_VIDEO_MODE_FORMAT7_0;
    dc1394color_coding_t colorCoding = DC1394_COLOR_CODING_RAW8;
    uint32_t left = 0;
    uint32_t top = 0;
    uint32_t imageWidth = 2440;
    uint32_t imageHeight = 2046;
    float fps = 60;
    uint32_t nDma = 4;
    bool externalTrigger = false;

    Dc1394Driver driver(guid, videoMode, colorCoding, left, top, imageWidth, imageHeight, fps, nDma, externalTrigger);

    CameraSample cameraSample;
    int counter = 0;
    while (!driver.capture(cameraSample) && counter < 10) {
        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    EXPECT_EQ(imageWidth, cameraSample.image().cols());
    EXPECT_EQ(imageHeight, cameraSample.image().rows());

    FILE* imagefile;
    imagefile = fopen("image.pgm", "wb");
    if (imagefile == NULL) {
        perror("Can't create output file");
    }
    uint32_t width = cameraSample.image().cols();
    uint32_t height = cameraSample.image().rows();
    fprintf(imagefile, "P5\n%u %u\n255\n", width, height);
    const unsigned char* imagePtr = (const unsigned char*)(cameraSample.image().data().data());
    fwrite(imagePtr, 1, height * width * 1, imagefile);
    fclose(imagefile);
}

TEST_F(HalDc1394DriverTest, canCaptureImagesWithTrig) {
    uint64_t guid = testGuid;
    dc1394video_mode_t videoMode = DC1394_VIDEO_MODE_FORMAT7_0;
    dc1394color_coding_t colorCoding = DC1394_COLOR_CODING_RAW8;
    uint32_t left = 0;
    uint32_t top = 0;
    uint32_t imageWidth = 2440;
    uint32_t imageHeight = 2046;
    float fps = 60;
    uint32_t nDma = 4;
    bool externalTrigger = true;

    Dc1394Driver driver(guid, videoMode, colorCoding, left, top, imageWidth, imageHeight, fps, nDma, externalTrigger);

    CameraSample cameraSample;
    int counter = 0;
    while (!driver.capture(cameraSample) && counter < 10) {
        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    EXPECT_EQ(imageWidth, cameraSample.image().cols());
    EXPECT_EQ(imageHeight, cameraSample.image().rows());

    FILE* imagefile;
    imagefile = fopen("image.pgm", "wb");
    if (imagefile == NULL) {
        perror("Can't create output file");
    }
    uint32_t width = cameraSample.image().cols();
    uint32_t height = cameraSample.image().rows();
    fprintf(imagefile, "P5\n%u %u\n255\n", width, height);
    const unsigned char* imagePtr = (const unsigned char*)(cameraSample.image().data().data());
    fwrite(imagePtr, 1, height * width * 1, imagefile);
    fclose(imagefile);
}
