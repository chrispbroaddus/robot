#pragma once
#include "camera_output_addresses.h"
#include "camera_publisher.h"
#include "packages/hal/proto/camera_id.pb.h"
#include "packages/unity_plugins/utils/include/zippy_image_interop.h"
#include <map>
#include <memory>

namespace unity_plugins {
///
/// @brief Holds all the camera publishers
///
class CameraPublisherGroup {
public:
    ///
    /// @brief Construct the Camera Publisher Group to manage sending the various rendered simulated image data to the clients
    ///
    /// @params[in] context : shared zmq context
    /// @params[in] addresses : Addresses for the camera outputs.
    /// @params[in] zmqLingerTimeInMilliSeconds : zmq l;inger time for publisher
    /// @params[in] zmqHighWaterMarkValue : Port high water mark value for publisher
    ///
    CameraPublisherGroup(std::shared_ptr<zmq::context_t> context, const std::map<hal::CameraId, CameraOutputAddresses>& cameraAddresses,
        int zmqLingerTimeInMilliSeconds, int zmqHighWaterMarkValue);

    ~CameraPublisherGroup() = default;

    ///
    /// @brief Send raw camera image data through ProtoBuffer serialization & ZMQ
    /// Will be routed to the correct camera publisher
    ///
    /// @params[in] cameraImage
    ///
    void sendImage(const CameraImage& cameraImage);

private:
    std::map<hal::CameraId, std::unique_ptr<CameraPublisher> > m_publishers;
};
} // namespace unity_plugins
