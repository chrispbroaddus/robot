#include "packages/hal/include/drivers/cameras/flycapture/flycapture_driver_factory.h"
#include "gtest/gtest.h"

using namespace hal;

TEST(HalFlycaptureDriverFactoryTest, canInstantiate) { EXPECT_NO_THROW(FlycaptureDriverFactory flycaptureDriverFactory()); }

TEST(HalFlycaptureDriverFactoryTest, noSerialNumber) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noGrabMode) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noCaptureTimeout) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noFormat7Mode) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noPixelFormat) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noWidth) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noHeight) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noleft) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noTop) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noNumBuffers) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noFps) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noExternalTrigger) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noAERoILeft) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noAERoITop) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noAERoIWidth) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, noAERoIHeight) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(16401251);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";

    EXPECT_THROW(flycaptureDriverFactory.create(deviceConfig), std::runtime_error);
}

TEST(HalFlycaptureDriverFactoryTest, canCreateCamera) {
    FlycaptureDriverFactory flycaptureDriverFactory;

    FlyCapture2::Error error;
    FlyCapture2::BusManager busManager;
    unsigned int numCameras = 0;
    uint32_t serialNumber = 0;

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

    error = busManager.GetCameraSerialNumberFromIndex(0, &serialNumber);
    if (error != FlyCapture2::PGRERROR_OK) {
        LOG(ERROR) << "Cannot get serial number of connected camera";
        throw std::runtime_error("Cannot get serial number of connected camera");
    }

    hal::details::property_map_t deviceConfig;
    deviceConfig["serialNumber"] = std::to_string(serialNumber);
    deviceConfig["grabMode"] = "DROP_FRAMES";
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(10);
    deviceConfig["format7Mode"] = "MODE_0";
    deviceConfig["pixelFormat"] = "MONO8";
    deviceConfig["width"] = std::to_string(2448);
    deviceConfig["height"] = std::to_string(2048);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["numBuffers"] = std::to_string(10);
    deviceConfig["fps"] = std::to_string(30);
    deviceConfig["externalTrigger"] = "OFF";
    deviceConfig["AERoILeft"] = "0";
    deviceConfig["AERoITop"] = "0";
    deviceConfig["AERoIWidth"] = "2448";
    deviceConfig["AERoIHeight"] = "2048";

    EXPECT_NO_THROW(flycaptureDriverFactory.create(deviceConfig));
}
