#include "packages/unity_plugins/simulated_cameras/include/camera_publisher.h"
#include "glog/logging.h"
#include "packages/core/include/chrono.h"
#include "packages/core/proto/timestamp.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/unity_plugins/simulated_cameras/include/simulator_publisher_topics.h"

#include <chrono>

using namespace unity_plugins;

CameraPublisher::CameraPublisher(std::shared_ptr<zmq::context_t> context, const CameraOutputAddresses& addresses,
    const int zmqLingerTimeInMilliSeconds, const int zmqHighWaterMarkValue)
    : m_context(context)
    , m_frameCount(0)
    , m_imagePublisher(*m_context, addresses.image, zmqHighWaterMarkValue, zmqLingerTimeInMilliSeconds)
    , m_depthPublisher(*m_context, addresses.depth, zmqHighWaterMarkValue, zmqLingerTimeInMilliSeconds)
    , m_pointcloudPublisher(*m_context, addresses.pointcloud, zmqHighWaterMarkValue, zmqLingerTimeInMilliSeconds) {}

void CameraPublisher::sendImage(const CameraImage& cameraImage) {

    hal::Format format;
    hal::Type type;
    hal::CameraId cameraId = cameraImage.cameraId;

    net::ZMQProtobufPublisher<hal::CameraSample>* publisher;

    switch (cameraImage.imageType) {
    case ImageType::Color:
        format = hal::Format::PB_RGB;
        type = hal::Type::PB_UNSIGNED_BYTE;
        publisher = &m_imagePublisher;
        break;
    case ImageType::Greyscale:
        format = hal::Format::PB_LUMINANCE;
        type = hal::Type::PB_UNSIGNED_BYTE;
        publisher = &m_imagePublisher;
        break;
    case ImageType::Depth:
        format = hal::Format::PB_LUMINANCE;
        type = hal::Type::PB_FLOAT;
        publisher = &m_depthPublisher;
        break;
    case ImageType::Pointcloud:
        format = hal::Format::PB_POINTCLOUD;
        type = hal::Type::PB_FLOAT;
        publisher = &m_pointcloudPublisher;
        break;
    default:
        LOG(ERROR) << "CameraPublisher::sendCameraSample Unknown image type";
        return;
    }

    hal::CameraSample cameraSample;

    auto device = new hal::Device();
    device->set_name(hal::CameraId_Name(cameraId));
    device->set_serialnumber(0);

    cameraSample.set_allocated_device(device);

    cameraSample.set_id(++m_frameCount);
    hal::Image* halImage = new hal::Image();
    halImage->set_rows(cameraImage.height);
    halImage->set_cols(cameraImage.width);

    int stride = cameraImage.width * cameraImage.bytesPerPixel;
    halImage->set_stride(stride);

    if (!cameraImage.data) {
        LOG(ERROR) << "Error: null pointer for imageData.";
        return;
    }

    size_t numBytes = stride * cameraImage.height;
    auto data = reinterpret_cast<char* const>(cameraImage.data);
    // TODO Do we need to copy the image data into a string or can we set directly. whaat happens with protobuffer and with zmq?
    // When is it safe for the cameraImage and its data to be deallocated or overwritten
    const std::string dataStr(data, numBytes);

    halImage->set_data(dataStr);
    halImage->set_type(type);
    halImage->set_format(format);

    auto gpsTimestamp = core::chrono::gps::wallClockInNanoseconds();
    auto systemTimestamp = new core::SystemTimestamp();
    CHECK_NOTNULL(systemTimestamp);
    systemTimestamp->set_nanos(gpsTimestamp.count());

    auto hardwareTimestamp = new core::HardwareTimestamp();
    CHECK_NOTNULL(hardwareTimestamp);
    hardwareTimestamp->set_nanos(gpsTimestamp.count());

    cameraSample.set_allocated_systemtimestamp(systemTimestamp);
    cameraSample.set_allocated_hardwaretimestamp(hardwareTimestamp);
    cameraSample.set_allocated_image(halImage);

    bool sendSuccess = publisher->send(cameraSample, CAMERA_OUTPUT_TOPIC);
    CHECK(sendSuccess);
}
