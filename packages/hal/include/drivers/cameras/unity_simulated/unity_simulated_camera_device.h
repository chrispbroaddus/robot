#pragma once

#include "packages/hal/include/drivers/cameras/camera_device_interface.h"
#include "packages/hal/include/hal_types.h"
#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/hal/proto/simulator_command_envelope.pb.h"
#include "packages/hal/proto/simulator_command_response.pb.h"
#include "packages/net/include/zmq_req_client.h"
#include "packages/net/include/zmq_topic_sub.h"

#include <zmq.hpp>

namespace hal {

class UnitySimulatedCameraDevice : public CameraDeviceInterface {
public:
    typedef net::ZMQProtobufSubscriber<CameraSample> camera_sample_sub_t;
    typedef net::ZMQProtobufReqClient<SimulatorCommandEnvelope, SimulatorCommandResponse> unity_camera_req_t;

    UnitySimulatedCameraDevice(const uint32_t id, const details::property_map_t& config);
    ~UnitySimulatedCameraDevice() = default;
    UnitySimulatedCameraDevice(const UnitySimulatedCameraDevice&) = delete;
    UnitySimulatedCameraDevice(const UnitySimulatedCameraDevice&&) = delete;
    UnitySimulatedCameraDevice& operator=(const UnitySimulatedCameraDevice&) = delete;
    UnitySimulatedCameraDevice& operator=(const UnitySimulatedCameraDevice&&) = delete;

    std::string deviceName() const override { return "UnitySimulatedCameraDevice"; }

    uint64_t serialNumber() const override { return m_cameraId; }

    bool capture(CameraSample& cameraSample) override;
    bool setAutoExposureRoi(float /*xFraction*/, float /*yFraction*/, float /*radiusFraction*/) { return false; }

private:
    zmq::context_t m_context = zmq::context_t(1);
    std::unique_ptr<camera_sample_sub_t> m_cameraSampleSubscriber;
    const uint32_t m_cameraId;
    int m_captureTimeoutInMilliseconds;
};
}
