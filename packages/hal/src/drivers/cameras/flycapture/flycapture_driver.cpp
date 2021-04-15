#include "packages/hal/include/drivers/cameras/flycapture/flycapture_driver.h"
#include "glog/logging.h"

#include "flycapture/FlyCapture2.h"

#include <cmath>

using namespace FlyCapture2;

namespace hal {

/// Camera Register addresses related to Auto-Exposure RoI control
constexpr static unsigned int AE_ROI_CTRL_ADDR = 0x1A70;
constexpr static unsigned int AE_ROI_OFFSET_ADDR = 0x1A74;

/// Camera Trigger source
enum triggerSource { GPIO_0 = 0, GPIO_1 = 1, GPIO_2 = 2, GPIO_3 = 3, SOFTWARE_TRIGGER = 7 };

void flycaptureLogError(const Error& error, const std::string& errorMessage) {
    error.PrintErrorTrace();
    LOG(ERROR) << error.GetDescription() << " in file: " << error.GetFilename() << " at line number: " << error.GetLine();
    LOG(ERROR) << errorMessage;
    throw std::runtime_error(errorMessage);
}

void flycaptureLogAndThrowError(const Error& error, const std::string& errorMessage) {
    error.PrintErrorTrace();
    LOG(ERROR) << error.GetDescription() << " in file: " << error.GetFilename() << " at line number: " << error.GetLine();
    LOG(ERROR) << errorMessage;
    throw std::runtime_error(errorMessage);
}

std::string getGrabModeString(GrabMode grabMode) {
    std::stringstream stream;
    switch (grabMode) {
    case DROP_FRAMES:
        stream << "DROP_FRAMES";
        break;
    case BUFFER_FRAMES:
        stream << "BUFFER_FRAMES";
        break;
    default:
        stream << "INVALID GRAB MODE";
    }
    return stream.str();
}

std::string getFormat7ModeString(Mode format7Mode) {
    std::stringstream stream;
    switch (format7Mode) {
    case MODE_0:
        stream << "MODE_0";
        break;
    case MODE_1:
        stream << "MODE_1";
        break;
    case MODE_2:
        stream << "MODE_2";
        break;
    case MODE_5:
        stream << "MODE_5";
        break;
    default:
        stream << "INVALID FORMAT7 MODE";
    }
    return stream.str();
}

std::string getPixelFormatString(PixelFormat pixelFormat) {
    std::stringstream stream;
    switch (pixelFormat) {
    case PIXEL_FORMAT_MONO8:
        stream << "PIXEL_FORMAT_MONO8";
        break;
    case PIXEL_FORMAT_RAW8:
        stream << "PIXEL_FORMAT_RAW8";
        break;
    case PIXEL_FORMAT_RGB8:
        stream << "PIXEL_FORMAT_RGB8";
        break;
    default:
        stream << "INVALID PIXEL FORMAT";
    }
    return stream.str();
}

void FlycaptureDriver::connectCamera() {

    Error error;
    BusManager busManager;
    unsigned int numCameras;
    PGRGuid guid;

    /// Get number of cameras connected
    error = busManager.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(error, "Cannot get number of connected cameras");
    }
    if (numCameras == 0) {
        LOG(ERROR) << "No cameras connected";
        throw std::runtime_error("No cameras connected");
    }

    /// Get the camera with the specified serial number if it is connected
    error = busManager.GetCameraFromSerialNumber(m_serialNumber, &guid);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(error, "Cannot find camera with serial number: " + std::to_string(m_serialNumber));
    }

    /// Connect to the camera
    error = m_camera.Connect(&guid);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(error, "Cannot connect to camera with serial number: " + std::to_string(m_serialNumber));
    }
}

void FlycaptureDriver::setTriggerMode(bool externalTrigger) {

    Error error;
    TriggerMode triggerMode;

    /// Retrieve current trigger settings from the camera
    error = m_camera.GetTriggerMode(&triggerMode);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(
            error, "Cannot get current trigger settings for camera with serial number: " + std::to_string(m_serialNumber));
    }

    /// Setup camera to accept external triggers if externalTrigger is enabled
    triggerMode.onOff = externalTrigger;
    triggerMode.mode = 0;
    triggerMode.parameter = 0;
    triggerMode.source = GPIO_3;

    /// Set the trigger settings on the camera
    error = m_camera.SetTriggerMode(&triggerMode);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(error, "Cannot set trigger settings for camera with serial number: " + std::to_string(m_serialNumber));
    }
}

void FlycaptureDriver::setCameraConfiguration(const GrabMode& grabMode, uint32_t numBuffers, uint32_t captureTimeoutInMilliseconds) {

    Error error;
    FC2Config config;

    /// Get the camera configuration
    error = m_camera.GetConfiguration(&config);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(error, "Cannot get configuration for camera with serial number: " + std::to_string(m_serialNumber));
    }

    /// Set the number of buffers used to grab images
    config.numBuffers = numBuffers;
    /// Time in milliseconds that RetrieveBuffer() and WaitForBufferEvent() will wait for an image before timing out and re-turning.
    config.grabTimeout = captureTimeoutInMilliseconds;
    /// The grab strategy employed during image transfer
    /// DROP_FRAMES Grabs newest image in the user buffer. Grabbing blocks if camera hasn't finished transmitting the next available image
    /// BUFFER_FRAMES Images accumulate in the user buffer, and the oldest image is grabbed for handling before being discarded
    config.grabMode = grabMode;
    /// Enables RetrieveBuffer to run in high performance mode. This means that any changes to the Bayer Tile or to the Embedded image info
    /// after StartCapture() will not be tracked when made using direct register writes
    config.highPerformanceRetrieveBuffer = true;

    /// Set the camera configuration
    error = m_camera.SetConfiguration(&config);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(error, "Cannot set configuration for camera with serial number: " + std::to_string(m_serialNumber));
    }
}

void FlycaptureDriver::configureImageSettings(
    const PixelFormat& pixelFormat, const Mode& format7Mode, uint32_t left, uint32_t top, uint32_t /*width*/, uint32_t /*height*/) {

    Error error;
    if (m_pixelFormat == FlyCapture2::PIXEL_FORMAT_MONO8 || m_pixelFormat == FlyCapture2::PIXEL_FORMAT_RAW8) {
        m_bytesPerPixel = 1;
    } else if (m_pixelFormat == FlyCapture2::PIXEL_FORMAT_RGB || m_pixelFormat == FlyCapture2::PIXEL_FORMAT_RGB8) {
        m_bytesPerPixel = 3;
    } else {
        LOG(ERROR) << "Unsupported pixel format on camera with serial number: " << m_serialNumber;
        throw std::runtime_error("Unsupported pixel format on camera with serial number: " + std::to_string(m_serialNumber));
    }

    Format7ImageSettings fmt7ImageSettings;
    fmt7ImageSettings.mode = format7Mode;
    fmt7ImageSettings.offsetX = left;
    fmt7ImageSettings.offsetY = top;
    fmt7ImageSettings.width = m_imageWidth;
    fmt7ImageSettings.height = m_imageHeight;
    fmt7ImageSettings.pixelFormat = pixelFormat;

    bool valid;
    Format7PacketInfo fmt7PacketInfo;
    /// Validates Format7ImageSettings structure and returns valid packet size information if the image settings are valid.
    error = m_camera.ValidateFormat7Settings(&fmt7ImageSettings, &valid, &fmt7PacketInfo);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(
            error, "Cannot validate format7 settings on camera with serial number: " + std::to_string(m_serialNumber));
    }
    if (!valid) {
        // Settings are not valid
        LOG(ERROR) << "Format7 settings are not valid for camera with serial number: " + std::to_string(m_serialNumber);
        throw std::runtime_error("Format7 settings are not valid for camera with serial number: " + std::to_string(m_serialNumber));
    }

    // Set the settings to the camera
    error = m_camera.SetFormat7Configuration(&fmt7ImageSettings, fmt7PacketInfo.recommendedBytesPerPacket);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(error, "Cannot set format7 settings on camera with serial number: " + std::to_string(m_serialNumber));
    }
}

void FlycaptureDriver::setFramerate(float& fps) {

    Error error;
    PropertyInfo propInfo;

    /// Check if the camera supports the FRAME_RATE property
    propInfo.type = FRAME_RATE;
    error = m_camera.GetPropertyInfo(&propInfo);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(error, "Cannot get property info on camera with serial number: " + std::to_string(m_serialNumber));
    }

    if (propInfo.present) {
        Property prop;
        prop.type = FRAME_RATE;
        prop.absControl = true;
        if (fps > propInfo.absMax) {
            prop.absValue = propInfo.absMax;
            LOG(INFO) << "Specified framerate greater than maximum value. Setting framerate to max value: " << propInfo.absMax;
        } else {
            prop.absValue = fps;
        }
        prop.autoManualMode = false;
        prop.onOff = true;

        error = m_camera.SetProperty(&prop);
        if (error != PGRERROR_OK) {
            flycaptureLogAndThrowError(error, "Cannot set framerate on camera with serial number: " + std::to_string(m_serialNumber));
        }

        error = m_camera.GetProperty(&prop);
        if (error != PGRERROR_OK) {
            flycaptureLogAndThrowError(error, "Cannot get framerate on camera with serial number: " + std::to_string(m_serialNumber));
        }
        fps = prop.absValue;
    } else {
        LOG(INFO) << "The camera does not support the FRAME_RATE property";
    }
}

bool FlycaptureDriver::setAutoExposureRoI(uint32_t left, uint32_t top, uint32_t width, uint32_t height) {

    uint32_t regValue;
    Error error;

    /// Get address of the base register
    error = m_camera.ReadRegister(AE_ROI_OFFSET_ADDR, &regValue);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(
            error, "Cannot read AE_ROI_OFFSET register on camera with serial number: " + std::to_string(m_serialNumber));
    }
    uint32_t baseRegAddr = regValue * 4 - 0xF00000;

    /// Set RoI position (left and top)
    regValue = (left << 16) + top;
    error = m_camera.WriteRegister(baseRegAddr + 0x8, regValue);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(
            error, "Cannot write to roi position register on camera with serial number: " + std::to_string(m_serialNumber));
    }

    /// Set RoI size (width and height)
    regValue = (width << 16) + height;
    error = m_camera.WriteRegister(baseRegAddr + 0xC, regValue);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(
            error, "Cannot write to roi size register on camera with serial number: " + std::to_string(m_serialNumber));
    }

    /// Turn on Auto-Exposure RoI control
    regValue = 0x82000000;
    error = m_camera.WriteRegister(AE_ROI_CTRL_ADDR, regValue);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(
            error, "Cannot write to auto-exposure roi control register on camera with serial number: " + std::to_string(m_serialNumber));
    }

    LOG(INFO) << "Set camera exposure roi to left: " << left << " top: " << top << " width: " << width << " height: " << height
              << " on camera with serial number: " << m_serialNumber;
    return true;
}

FlycaptureDriver::FlycaptureDriver(uint64_t serialNumber, const GrabMode& grabMode, const Mode& format7Mode, const PixelFormat& pixelFormat,
    uint32_t left, uint32_t top, uint32_t width, uint32_t height, float& fps, uint32_t numBuffers, uint32_t captureTimeoutInMilliseconds,
    bool externalTrigger, uint32_t AERoILeft, uint32_t AERoITop, uint32_t AERoIWidth, uint32_t AERoIHeight)
    : m_camera()
    , m_serialNumber(serialNumber)
    , m_imageWidth(width)
    , m_imageHeight(height)
    , m_pixelFormat(pixelFormat)
    , m_defaultAERoILeft(AERoILeft)
    , m_defaultAERoITop(AERoITop)
    , m_defaultAERoIWidth(AERoIWidth)
    , m_defaultAERoIHeight(AERoIHeight)
    , m_frameCount(0)
    , m_bytesPerPixel(-1) {

    Error error;

    /// Connect to the camera with the specified serial number
    connectCamera();

    /// Configure external trigger
    setTriggerMode(externalTrigger);

    /// Configure image capture settings
    setCameraConfiguration(grabMode, numBuffers, captureTimeoutInMilliseconds);

    /// Configure image settings
    configureImageSettings(pixelFormat, format7Mode, left, top, width, height);

    /// Set framerate
    setFramerate(fps);

    /// Enables Auto-Exposure RoI Control and sets the RoI
    setAutoExposureRoI(m_defaultAERoILeft, m_defaultAERoITop, m_defaultAERoIWidth, m_defaultAERoIHeight);

    /// Start capturing images
    error = m_camera.StartCapture();
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(error, "Cannot start image capture on camera with serial number: " + std::to_string(m_serialNumber));
    }

    LOG(INFO) << "Initialized flycapture camera with: \n"
              << "serialNumber: " << serialNumber << std::endl
              << "grabMode: " << getGrabModeString(grabMode) << std::endl
              << "format7Mode: " << getFormat7ModeString(format7Mode) << std::endl
              << "pixelFormat: " << getPixelFormatString(pixelFormat) << std::endl
              << "left: " << left << std::endl
              << "top: " << top << std::endl
              << "width: " << m_imageWidth << std::endl
              << "height: " << m_imageHeight << std::endl
              << "bytesPerPixel: " << m_bytesPerPixel << std::endl
              << "nDma: " << numBuffers << std::endl
              << "fps: " << fps << std::endl
              << "externalTrigger: " << externalTrigger << std::endl
              << "AERoILeft: " << m_defaultAERoILeft << std::endl
              << "AERoITop: " << m_defaultAERoITop << std::endl
              << "AERoIWidth: " << m_defaultAERoIWidth << std::endl
              << "AERoIHeight: " << m_defaultAERoIHeight << std::endl;
}

std::string FlycaptureDriver::deviceName() const { return "Flycapture Camera"; }

uint64_t FlycaptureDriver::serialNumber() const { return m_serialNumber; }

bool FlycaptureDriver::capture(CameraSample& cameraSample) {

    Error error;
    FlyCapture2::Image rawImage;

    /// Capture an image
    error = m_camera.RetrieveBuffer(&rawImage);
    if (error != PGRERROR_OK) {
        if (error == PGRERROR_TIMEOUT) {
            return false;
        } else {
            flycaptureLogAndThrowError(error, "Cannot capture image using camera with serial number: " + std::to_string(m_serialNumber));
        }
    }

    CHECK_EQ(rawImage.GetCols(), m_imageWidth);
    CHECK_EQ(rawImage.GetRows(), m_imageHeight);
    CHECK_EQ(rawImage.GetStride(), m_imageWidth * m_bytesPerPixel);

    FlyCapture2::Image convertedImage;
    error = rawImage.Convert(m_pixelFormat, &convertedImage);
    if (error != PGRERROR_OK) {
        flycaptureLogAndThrowError(error, "Cannot convert image format using camera with serial number: " + std::to_string(m_serialNumber));
    }

    // Device
    hal::Device* device = new Device();
    device->set_name(deviceName());
    device->set_serialnumber(serialNumber());

    // Image
    hal::Image* image = new hal::Image();
    image->set_cols(convertedImage.GetCols());
    image->set_rows(convertedImage.GetRows());
    image->set_stride(convertedImage.GetStride());
    image->set_type(hal::PB_UNSIGNED_BYTE);
    if (m_pixelFormat == PIXEL_FORMAT_MONO8 || m_pixelFormat == PIXEL_FORMAT_RAW8) {
        image->set_format(hal::PB_LUMINANCE);
    } else if (m_pixelFormat == PIXEL_FORMAT_RGB) {
        image->set_format(hal::PB_RGB);
    }

    image->set_data(convertedImage.GetData(), m_imageWidth * m_imageHeight * m_bytesPerPixel);

    const std::chrono::nanoseconds timestamp = core::chrono::gps::wallClockInNanoseconds();

    // HardwareTimestamp
    core::HardwareTimestamp* hardwareTimestamp = new core::HardwareTimestamp();

    // SystemTimestamp
    core::SystemTimestamp* systemTimestamp = new core::SystemTimestamp();
    systemTimestamp->set_nanos(timestamp.count());

    // Set the CameraSample
    cameraSample.set_allocated_device(device);
    cameraSample.set_allocated_image(image);
    cameraSample.set_allocated_systemtimestamp(systemTimestamp);
    cameraSample.set_allocated_hardwaretimestamp(hardwareTimestamp);

    cameraSample.set_id(++m_frameCount);

    LOG_EVERY_N(INFO, 100) << "Captured frame: " << m_frameCount << " using camera with serialNumber: " << m_serialNumber;
    return true;
}

bool FlycaptureDriver::setAutoExposureRoi(float xFraction, float yFraction, float radiusFraction) {

    if (xFraction < 0 || xFraction > 1) {
        throw std::runtime_error("Set Auto-Exposure RoI: Invalid xFraction");
    } else if (yFraction < 0 || yFraction > 1) {
        throw std::runtime_error("Set Auto-Exposure RoI: Invalid yFraction");
    } else if (radiusFraction < 0 || radiusFraction > 1) {
        throw std::runtime_error("Set Auto-Exposure RoI: Invalid radiusFraction");
    }

    if (xFraction == 0 && yFraction == 0 && radiusFraction == 0) {
        return setAutoExposureRoI(m_defaultAERoILeft, m_defaultAERoITop, m_defaultAERoIWidth, m_defaultAERoIHeight);
    } else {
        int left = (int)std::round(xFraction * m_imageWidth - radiusFraction * m_imageWidth);
        left = left < 0 ? 0 : left;
        int right = (int)std::round(xFraction * m_imageWidth + radiusFraction * m_imageWidth);
        right = right > static_cast<int>(m_imageWidth) ? m_imageWidth : right;
        int top = (int)std::round(yFraction * m_imageHeight - radiusFraction * m_imageWidth);
        top = top < 0 ? 0 : top;
        int bottom = (int)std::round(yFraction * m_imageHeight + radiusFraction * m_imageWidth);
        bottom = bottom > static_cast<int>(m_imageHeight) ? m_imageHeight : bottom;

        return setAutoExposureRoI((uint32_t)left, (uint32_t)top, (uint32_t)(right - left), (uint32_t)(bottom - top));
    }
}

FlycaptureDriver::~FlycaptureDriver() {
    /// Stop capturing images
    Error error = m_camera.StopCapture();
    if (error != PGRERROR_OK) {
        flycaptureLogError(error, "Cannot stop image capture on camera with serial number: " + std::to_string(m_serialNumber));
    }
    LOG(INFO) << "Stopped image capture on camera with serial number: " << m_serialNumber;

    /// Disconnect the camera
    error = m_camera.Disconnect();
    if (error != PGRERROR_OK) {
        flycaptureLogError(error, "Cannot disconnect camera with serial number: " + std::to_string(m_serialNumber));
    }
    LOG(INFO) << "Disconnected camera with serial number: " << m_serialNumber;
}
}
