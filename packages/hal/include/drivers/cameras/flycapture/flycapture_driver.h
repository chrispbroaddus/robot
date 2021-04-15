#pragma once

#include "packages/core/include/chrono.h"
#include "packages/core/proto/timestamp.pb.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"

#include "flycapture/FlyCapture2.h"

namespace hal {

class FlycaptureDriver : public CameraDeviceInterface {
public:
    ///
    /// @brief Create a camera driver that uses Flycapture SDK to control the camera
    /// \param[in] serialNumber : Serial number of the camera to connect to
    /// \param[in] grabMode : Configure whether the camera should return latest frame or grab frames in order from the buffer
    /// \param[in] format7Mode : Camera video mode. Use MODE_0 for full resolution without binning
    /// \param[in] pixelFormat : Determines image color mode and depth
    /// \param[in] left : Leftmost point (in pixels) of the image RoI
    /// \param[in] top : Topmost point (in pixels) of the image RoI
    /// \param[in] width : Width in pixels of the image RoI
    /// \param[in] height : Height in pixels of the image RoI
    /// \param[in,out] fps : Camera framerate. If the requested framerate is too high, sets max framerate and returns value
    /// \param[in] numBuffers : Size of the ring buffer to use for image capture
    /// \param[in] captureTimeoutInMilliseconds : Capture timeout
    /// \param[in] externalTrigger : Boolean to turn external trigger on or off
    /// \param[in] AERoILeft : Auto Exposure RoI left
    /// \param[in] AERoITop : Auto Exposure RoI top
    /// \param[in] AERoIWidth : Auto Exposure RoI width
    /// \param[in] AERoIHeight : Auto Exposure RoI height
    FlycaptureDriver(uint64_t serialNumber, const FlyCapture2::GrabMode& grabMode, const FlyCapture2::Mode& format7Mode,
        const FlyCapture2::PixelFormat& pixelFormat, uint32_t left, uint32_t top, uint32_t width, uint32_t height, float& fps,
        uint32_t numBuffers, uint32_t captureTimeoutInMilliseconds, bool externalTrigger, uint32_t AERoILeft, uint32_t AERoITop,
        uint32_t AERoIWidth, uint32_t AERoIHeight);
    ~FlycaptureDriver();
    /// Returns the camera's device name
    std::string deviceName() const override;
    /// Returns the serial number of the camera
    uint64_t serialNumber() const override;
    /// Capture an image from the camera
    bool capture(CameraSample& cameraSample) override;
    /// Set Auto-Exposure RoI based on User Input
    bool setAutoExposureRoi(float xFraction, float yFraction, float radiusFraction) override;

private:
    /// Connect to the camera with the specified serial number
    void connectCamera();
    /// Configure external trigger
    void setTriggerMode(bool externalTrigger);
    /// Configure image capture settings
    void setCameraConfiguration(const FlyCapture2::GrabMode& grabMode, uint32_t numBuffers, uint32_t captureTimeoutInMilliseconds);
    /// Configure image settings
    void configureImageSettings(const FlyCapture2::PixelFormat& pixelFormat, const FlyCapture2::Mode& format7Mode, uint32_t left,
        uint32_t top, uint32_t width, uint32_t height);
    /// Set framerate
    void setFramerate(float& fps);
    /// Enables Auto-Exposure RoI Control and sets the RoI
    bool setAutoExposureRoI(uint32_t left, uint32_t top, uint32_t width, uint32_t height);

    FlyCapture2::Camera m_camera;
    const uint64_t m_serialNumber;
    const uint32_t m_imageWidth;
    const uint32_t m_imageHeight;
    const FlyCapture2::PixelFormat m_pixelFormat;
    const uint32_t m_defaultAERoILeft;
    const uint32_t m_defaultAERoITop;
    const uint32_t m_defaultAERoIWidth;
    const uint32_t m_defaultAERoIHeight;
    uint32_t m_frameCount;
    uint32_t m_bytesPerPixel;
};
}
