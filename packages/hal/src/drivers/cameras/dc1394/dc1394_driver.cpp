#include "packages/hal/include/drivers/cameras/dc1394/dc1394_driver.h"
#include "glog/logging.h"
#include "packages/core/include/chrono.h"
#include "packages/hal/include/string_utils.h"

#include <iostream>
#include <sstream>
#include <unistd.h>

#include <dc1394/dc1394.h>

namespace hal {

void sanityCheckInput(
    const uint64_t& guid, const dc1394video_mode_t& videoMode, const dc1394color_coding_t& colorCoding, const uint32_t& nDma) {
    LOG(INFO) << "Running Sanity Check on Input";
    if (guid == 0) {
        LOG(ERROR) << "Invalid GUID";
        throw std::runtime_error("Invalid GUID");
    }
    if (videoMode < DC1394_VIDEO_MODE_MIN || videoMode > DC1394_VIDEO_MODE_MAX) // ToDo: Check if supported
    {
        LOG(ERROR) << "Invalid video mode";
        throw std::runtime_error("Invalid video mode");
    }
    if (colorCoding < DC1394_COLOR_CODING_MIN || colorCoding > DC1394_COLOR_CODING_MAX) // ToDo: Check if supported
    {
        LOG(ERROR) << "Invalid color code";
        throw std::runtime_error("Invalid color code");
    }
    if (nDma == 0) {
        LOG(ERROR) << "Invalid num of DMA channels";
        throw std::runtime_error("Invalid num of DMA channels");
    }
    LOG(INFO) << "Completed Sanity Check on Input";
}

void dc1394ThrowError(dc1394error_t err, std::string message) {
    if ((err > 0) || (err <= -DC1394_ERROR_NUM)) {
        err = DC1394_INVALID_ERROR_CODE;
    }

    if (err != DC1394_SUCCESS) {
        dc1394_log_error("%s: in %s (%s, line %d): %s\n", dc1394_error_get_string(err), __FUNCTION__, __FILE__, __LINE__, message);

        throw std::runtime_error(message);
    }
}

void dc1394CleanAndThrowError(dc1394error_t err, dc1394camera_t* camera, std::string message) {
    if ((err > 0) || (err <= -DC1394_ERROR_NUM)) {
        err = DC1394_INVALID_ERROR_CODE;
    }

    if (err != DC1394_SUCCESS) {
        dc1394_log_error("%s: in %s (%s, line %d): %s\n", dc1394_error_get_string(err), __FUNCTION__, __FILE__, __LINE__, message);

        dc1394_video_set_transmission(camera, DC1394_OFF);
        dc1394_capture_stop(camera);
        dc1394_camera_free(camera);
        throw std::runtime_error(message);
    }
}

std::string printVideoMode(dc1394video_mode_t mode) {
    std::stringstream stream;
    switch (mode) {
    case DC1394_VIDEO_MODE_160x120_YUV444:
        stream << "DC1394_VIDEO_MODE_160x120_YUV444";
        break;
    case DC1394_VIDEO_MODE_320x240_YUV422:
        stream << "DC1394_VIDEO_MODE_320x240_YUV422";
        break;
    case DC1394_VIDEO_MODE_640x480_YUV411:
        stream << "DC1394_VIDEO_MODE_640x480_YUV411";
        break;
    case DC1394_VIDEO_MODE_640x480_YUV422:
        stream << "DC1394_VIDEO_MODE_640x480_YUV422";
        break;
    case DC1394_VIDEO_MODE_640x480_RGB8:
        stream << "DC1394_VIDEO_MODE_640x480_RGB8";
        break;
    case DC1394_VIDEO_MODE_640x480_MONO8:
        stream << "DC1394_VIDEO_MODE_640x480_MONO8";
        break;
    case DC1394_VIDEO_MODE_640x480_MONO16:
        stream << "DC1394_VIDEO_MODE_640x480_MONO16";
        break;
    case DC1394_VIDEO_MODE_800x600_YUV422:
        stream << "DC1394_VIDEO_MODE_800x600_YUV422";
        break;
    case DC1394_VIDEO_MODE_800x600_RGB8:
        stream << "DC1394_VIDEO_MODE_800x600_RGB8";
        break;
    case DC1394_VIDEO_MODE_800x600_MONO8:
        stream << "DC1394_VIDEO_MODE_800x600_MONO8";
        break;
    case DC1394_VIDEO_MODE_1024x768_YUV422:
        stream << "DC1394_VIDEO_MODE_1024x768_YUV422";
        break;
    case DC1394_VIDEO_MODE_1024x768_RGB8:
        stream << "DC1394_VIDEO_MODE_1024x768_RGB8";
        break;
    case DC1394_VIDEO_MODE_1024x768_MONO8:
        stream << "DC1394_VIDEO_MODE_1024x768_MONO8";
        break;
    case DC1394_VIDEO_MODE_800x600_MONO16:
        stream << "DC1394_VIDEO_MODE_800x600_MONO16";
        break;
    case DC1394_VIDEO_MODE_1024x768_MONO16:
        stream << "DC1394_VIDEO_MODE_1024x768_MONO16";
        break;
    case DC1394_VIDEO_MODE_1280x960_YUV422:
        stream << "DC1394_VIDEO_MODE_1280x960_YUV422";
        break;
    case DC1394_VIDEO_MODE_1280x960_RGB8:
        stream << "DC1394_VIDEO_MODE_1280x960_RGB8";
        break;
    case DC1394_VIDEO_MODE_1280x960_MONO8:
        stream << "DC1394_VIDEO_MODE_1280x960_MONO8";
        break;
    case DC1394_VIDEO_MODE_1600x1200_YUV422:
        stream << "DC1394_VIDEO_MODE_1600x1200_YUV422";
        break;
    case DC1394_VIDEO_MODE_1600x1200_RGB8:
        stream << "DC1394_VIDEO_MODE_1600x1200_RGB8";
        break;
    case DC1394_VIDEO_MODE_1600x1200_MONO8:
        stream << "DC1394_VIDEO_MODE_1600x1200_MONO8";
        break;
    case DC1394_VIDEO_MODE_1280x960_MONO16:
        stream << "DC1394_VIDEO_MODE_1280x960_MONO16";
        break;
    case DC1394_VIDEO_MODE_1600x1200_MONO16:
        stream << "DC1394_VIDEO_MODE_1600x1200_MONO16";
        break;
    case DC1394_VIDEO_MODE_EXIF:
        stream << "DC1394_VIDEO_MODE_EXIF";
        break;
    case DC1394_VIDEO_MODE_FORMAT7_0:
        stream << "DC1394_VIDEO_MODE_FORMAT7_0";
        break;
    case DC1394_VIDEO_MODE_FORMAT7_1:
        stream << "DC1394_VIDEO_MODE_FORMAT7_1";
        break;
    case DC1394_VIDEO_MODE_FORMAT7_2:
        stream << "DC1394_VIDEO_MODE_FORMAT7_2";
        break;
    case DC1394_VIDEO_MODE_FORMAT7_3:
        stream << "DC1394_VIDEO_MODE_FORMAT7_3";
        break;
    case DC1394_VIDEO_MODE_FORMAT7_4:
        stream << "DC1394_VIDEO_MODE_FORMAT7_4";
        break;
    case DC1394_VIDEO_MODE_FORMAT7_5:
        stream << "DC1394_VIDEO_MODE_FORMAT7_5";
        break;
    case DC1394_VIDEO_MODE_FORMAT7_6:
        stream << "DC1394_VIDEO_MODE_FORMAT7_6";
        break;
    case DC1394_VIDEO_MODE_FORMAT7_7:
        stream << "DC1394_VIDEO_MODE_FORMAT7_7";
        break;
    default:
        stream << "INVALID DC1394 VIDEO MODE";
    }
    return stream.str();
}

std::string printColorCoding(dc1394color_coding_t colorCoding) {
    std::stringstream stream;
    switch (colorCoding) {
    case DC1394_COLOR_CODING_MONO8:
        stream << "DC1394_COLOR_CODING_MONO8";
        break;
    case DC1394_COLOR_CODING_YUV411:
        stream << "DC1394_COLOR_CODING_YUV411";
        break;
    case DC1394_COLOR_CODING_YUV422:
        stream << "DC1394_COLOR_CODING_YUV422";
        break;
    case DC1394_COLOR_CODING_YUV444:
        stream << "DC1394_COLOR_CODING_YUV444";
        break;
    case DC1394_COLOR_CODING_RGB8:
        stream << "DC1394_COLOR_CODING_RGB8";
        break;
    case DC1394_COLOR_CODING_RGB16:
        stream << "DC1394_COLOR_CODING_RGB16";
        break;
    case DC1394_COLOR_CODING_MONO16:
        stream << "DC1394_COLOR_CODING_MONO16";
        break;
    case DC1394_COLOR_CODING_MONO16S:
        stream << "DC1394_COLOR_CODING_MONO16S";
        break;
    case DC1394_COLOR_CODING_RGB16S:
        stream << "DC1394_COLOR_CODING_RGB16S";
        break;
    case DC1394_COLOR_CODING_RAW8:
        stream << "DC1394_COLOR_CODING_RAW8";
        break;
    case DC1394_COLOR_CODING_RAW16:
        stream << "DC1394_COLOR_CODING_RAW16";
        break;
    default:
        stream << "INVALID DC1394 COLOR CODING";
    }
    return stream.str();
}

Dc1394Driver::Dc1394Driver(const uint64_t& guid, const dc1394video_mode_t& videoMode, const dc1394color_coding_t& colorCoding,
    const uint32_t& left, const uint32_t& top, const uint32_t& width, const uint32_t& height, const float& fps, const uint32_t& nDma,
    const bool externalTrigger)
    : imageWidth(width)
    , imageHeight(height)
    , frameCount(0) {
    LOG(INFO) << "Initializing dc1394 driver";

    sanityCheckInput(guid, videoMode, colorCoding, nDma);

    dc1394error_t err;
    dc1394camera_list_t* list;
    dc1394feature_info_t featureInfo;
    dc1394bool_t polarityCapable;
    float setFps;
    uint32_t cameraIdx = 0;

    bus = dc1394_new();
    if (!bus) {
        dc1394_log_error("Dc1394: Bus creation failed");
        throw std::runtime_error("Dc1394: Bus creation failed");
    }

    err = dc1394_camera_enumerate(bus, &list);
    dc1394ThrowError(err, "Dc1394: Camera Enumeration failed");
    if (list->num == 0) {
        dc1394_log_error("No cameras found");
        throw std::runtime_error("No cameras found");
    }

    for (cameraIdx = 0; cameraIdx < list->num; cameraIdx++) {
        if (list->ids[cameraIdx].guid == guid) {
            break;
        }
    }
    if (cameraIdx == list->num) {
        std::stringstream ssMsg;
        std::string sMsg;
        ssMsg << "No camera with guid: " << guid << " found";
        sMsg = ssMsg.str();
        dc1394_log_error(sMsg.c_str());
        throw std::runtime_error(sMsg.c_str());
    }

    camera = dc1394_camera_new(bus, list->ids[cameraIdx].guid);
    if (!camera) {
        std::stringstream ssMsg;
        std::string sMsg;
        ssMsg << "Failed to initialize camera with guid: " << list->ids[cameraIdx].guid;
        sMsg = ssMsg.str();
        dc1394_log_error(sMsg.c_str());
        throw std::runtime_error(sMsg.c_str());
    }
    dc1394_camera_free_list(list);

    err = dc1394_reset_bus(camera);
    dc1394CleanAndThrowError(err, camera, "Could not reset bus");
    usleep(10000);

    err = dc1394_camera_reset(camera);
    dc1394CleanAndThrowError(err, camera, "Could not reset camera");
    usleep(10000);

    err = dc1394_video_set_operation_mode(camera, DC1394_OPERATION_MODE_1394B);
    dc1394CleanAndThrowError(err, camera, "Could not set operation mode");

    err = dc1394_video_set_mode(camera, videoMode);
    dc1394CleanAndThrowError(err, camera, "Could not set video mode");

    if (videoMode < DC1394_VIDEO_MODE_FORMAT7_MIN) // ToDo: Implement for completeness
    {
        dc1394_log_error("Video Mode not supported.\n");

    } else {
        dc1394format7mode_t modeInfo;
        modeInfo.present = DC1394_TRUE;

        err = dc1394_format7_get_mode_info(camera, videoMode, &modeInfo);
        dc1394CleanAndThrowError(err, camera, "Could not fetch FORMAT7 mode information");

        err = dc1394_format7_set_packet_size(camera, videoMode, modeInfo.max_packet_size);
        dc1394CleanAndThrowError(err, camera, "Could not set packet size");

        err = dc1394_format7_set_roi(camera, videoMode, colorCoding, DC1394_USE_RECOMMENDED, left, top, imageWidth, imageHeight);
        dc1394CleanAndThrowError(err, camera, "Could not set image roi");

        if (fps > 0) {
            featureInfo.id = DC1394_FEATURE_FRAME_RATE;
            err = dc1394_feature_get(camera, &featureInfo);

            err = dc1394_feature_set_absolute_control(camera, DC1394_FEATURE_FRAME_RATE, DC1394_ON);
            dc1394CleanAndThrowError(err, camera, "Could not turn on absolute control for framerate");

            err = dc1394_feature_set_mode(camera, DC1394_FEATURE_FRAME_RATE, DC1394_FEATURE_MODE_MANUAL);
            dc1394CleanAndThrowError(err, camera, "Could not make framerate control manual");

            if (fps < featureInfo.abs_max) {
                err = dc1394_feature_set_absolute_value(camera, DC1394_FEATURE_FRAME_RATE, fps);
                dc1394CleanAndThrowError(err, camera, "Could not set framerate value");
            } else {
                LOG(WARNING) << "Specified framerate greater than max framerate. Setting framerate to the max value: "
                             << featureInfo.abs_max << " fps";
                err = dc1394_feature_set_absolute_value(camera, DC1394_FEATURE_FRAME_RATE, featureInfo.abs_max);
                dc1394CleanAndThrowError(err, camera, "Could not set framerate value");
            }
        }
        err = dc1394_feature_get_absolute_value(camera, DC1394_FEATURE_FRAME_RATE, &setFps);
        dc1394CleanAndThrowError(err, camera, "Could not get framerate value");
    }

    if (colorCoding == DC1394_COLOR_CODING_MONO8 || colorCoding == DC1394_COLOR_CODING_RAW8) {
        bytesPerPixel = 1;
    } else if (colorCoding == DC1394_COLOR_CODING_RGB8) {
        bytesPerPixel = 3;
    }

    if (externalTrigger) {
        err = dc1394_external_trigger_has_polarity(camera, &polarityCapable);
        dc1394CleanAndThrowError(err, camera, "Could not check trigger polarity switching capability");

        if (polarityCapable) {
            err = dc1394_external_trigger_set_polarity(camera, DC1394_TRIGGER_ACTIVE_HIGH);
            dc1394CleanAndThrowError(err, camera, "Could not set trigger polarity.");
        }

        err = dc1394_external_trigger_set_mode(camera, DC1394_TRIGGER_MODE_0);
        dc1394CleanAndThrowError(err, camera, "Could not set trigger mode.");

        err = dc1394_external_trigger_set_source(camera, DC1394_TRIGGER_SOURCE_3);
        dc1394CleanAndThrowError(err, camera, "Could not set trigger source.");

        err = dc1394_external_trigger_set_power(camera, DC1394_ON);
        dc1394CleanAndThrowError(err, camera, "Could not set trigger source.");
    }

    err = dc1394_capture_setup(camera, nDma, DC1394_CAPTURE_FLAGS_DEFAULT);
    dc1394CleanAndThrowError(err, camera, "Could not setup image capture");

    err = dc1394_video_set_transmission(camera, DC1394_ON);
    dc1394CleanAndThrowError(err, camera, "Could not set video transmission");

    LOG(INFO) << "Initialized dc1394 driver with: \n"
              << "Camera Pointer: " << camera << std::endl
              << "GUID: " << guid << std::endl
              << "videoMode: " << printVideoMode(videoMode) << std::endl
              << "colorCoding: " << printColorCoding(colorCoding) << std::endl
              << "left: " << left << std::endl
              << "top: " << top << std::endl
              << "width: " << imageWidth << std::endl
              << "height: " << imageHeight << std::endl
              << "bytesPerPixel: " << bytesPerPixel << std::endl
              << "nDma: " << nDma << std::endl
              << "fps: " << setFps << std::endl
              << "frameCount: " << frameCount << std::endl;
}

std::string Dc1394Driver::deviceName() const { return std::string(camera->model); }

uint64_t Dc1394Driver::serialNumber() const { return camera->guid; }

bool Dc1394Driver::capture(CameraSample& cameraSample) {
    LOG(INFO) << "Capturing frame: " << frameCount + 1 << " using camera: " << camera;
    dc1394error_t err;
    dc1394video_frame_t* frame = NULL;

    err = dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &frame);
    if (err != DC1394_SUCCESS) {
        LOG(ERROR) << "Capture frame: " << frameCount + 1 << " using camera: " << camera << "failed.";
        return false;
    }
    if (frame == NULL) {
        LOG(INFO) << "Capture timeout. Camera: " << camera << " has no frame available";
        return false;
    }

    // Device
    hal::Device* device = new Device();
    device->set_name(deviceName());
    device->set_serialnumber(serialNumber());

    // Image
    hal::Image* image = new hal::Image();
    image->set_cols(imageWidth);
    image->set_rows(imageHeight);
    image->set_stride(imageWidth * bytesPerPixel);
    image->set_type(hal::PB_UNSIGNED_BYTE);
    if (frame->color_coding == DC1394_COLOR_CODING_MONO8 || frame->color_coding == DC1394_COLOR_CODING_RAW8) {
        image->set_format(hal::PB_LUMINANCE);
    } else if (frame->color_coding == DC1394_COLOR_CODING_RGB8) {
        image->set_format(hal::PB_RGB);
    }

    image->set_data(frame->image, imageWidth * imageHeight * bytesPerPixel);

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

    cameraSample.set_id(++frameCount);

    dc1394_capture_enqueue(camera, frame);

    LOG(INFO) << "Captured frame: " << frameCount << " using camera: " << camera;
    return true;
}

Dc1394Driver::~Dc1394Driver() {
    if (camera != NULL) {
        dc1394_video_set_transmission(camera, DC1394_OFF);
        LOG(INFO) << "Video transmission turned off";
        dc1394_capture_stop(camera);
        LOG(INFO) << "Stopped capture";
        dc1394_camera_free(camera);
        LOG(INFO) << "Freed camera";
        camera = NULL;
    }
    if (bus != NULL) {
        dc1394_free(bus);
        LOG(INFO) << "Freed bus";
        bus = NULL;
    }
}
}
