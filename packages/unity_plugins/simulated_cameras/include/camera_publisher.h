#pragma once

#include "camera_output_addresses.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/net/include/zmq_topic_pub.h"
#include "packages/unity_plugins/utils/include/zippy_image_interop.h"
#include "simulator_network_interop.h"
#include <memory>
#include <string>

namespace unity_plugins {

///
/// @brief Publishes images using zmq and protobuffer serialization
///
class CameraPublisher {

public:
    ///
    /// @brief Construct the Camera Publisher to send rendered simulated image data to the client
    ///
    /// @params[in] context : shared zmq context
    /// @params[in] addresses : Addresses for the camera outputs.
    /// @params[in] zmqLingerTimeInMilliSeconds : zmq l;inger time for publisher
    /// @params[in] zmqHighWaterMarkValue : Port high water mark value for publisher
    ///
    CameraPublisher(std::shared_ptr<zmq::context_t> context, const CameraOutputAddresses& addresses, const int zmqLingerTimeInMilliSeconds,
        const int zmqHighWaterMarkValue);

    ///
    /// @brief Copy constructor
    ///
    CameraPublisher(const CameraPublisher& obj) = delete;

    ///
    /// @brief Copy assignment constructor
    ///
    CameraPublisher& operator=(CameraPublisher other) = delete;

    ///
    /// @brief Destructor
    ///
    ~CameraPublisher() = default;

    ///
    /// @brief Send raw camera image data through ProtoBuffer serialization & ZMQ
    /// Routes the image to the correct publisher
    ///
    /// @params[in] cameraImage
    ///
    void sendImage(const CameraImage& cameraImage);

private:
    std::shared_ptr<zmq::context_t> m_context;
    uint32_t m_frameCount;
    net::ZMQProtobufPublisher<hal::CameraSample> m_imagePublisher;
    net::ZMQProtobufPublisher<hal::CameraSample> m_depthPublisher;
    net::ZMQProtobufPublisher<hal::CameraSample> m_pointcloudPublisher;
};
} // namespace unity_plugins
