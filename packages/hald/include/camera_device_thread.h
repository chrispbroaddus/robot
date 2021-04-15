
#pragma once

#include "packages/hald/include/device_thread.h"
#include "packages/hald/proto/device_config.pb.h"

#include "packages/hal/include/drivers/cameras/camera_device_interface.h"
#include "packages/hal/proto/auto_exposure_roi_sample.pb.h"
#include "packages/hal/proto/camera_sample.pb.h"

#include "packages/net/include/zmq_rep_server.h"
#include "packages/net/include/zmq_topic_pub.h"

namespace hald {

class CameraDeviceThread : public DeviceThread {
public:
    CameraDeviceThread(const Device& deviceConfig);
    ~CameraDeviceThread();

    void run() override;

private:
    typedef net::ZMQProtobufPublisher<hal::CameraSample> camera_pub_t;
    typedef net::ZMQProtobufRepServer<hal::AutoExposureRoiCommand, hal::AutoExposureRoiResponse> ae_roi_server_t;

    zmq::context_t m_context;
    std::string m_topic;
    std::shared_ptr<hal::CameraDeviceInterface> m_camera;
    std::unique_ptr<camera_pub_t> m_cameraSamplePublisher;
    std::unique_ptr<ae_roi_server_t> m_aeRoiServer;
};
}
