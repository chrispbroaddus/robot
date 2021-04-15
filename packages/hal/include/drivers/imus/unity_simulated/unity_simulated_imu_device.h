#pragma once

#include "packages/hal/include/drivers/imus/imu_device_interface.h"
#include "packages/hal/include/hal_types.h"
#include "packages/hal/proto/imu_sample.pb.h"
#include "packages/net/include/zmq_topic_sub.h"

#include <zmq.hpp>

namespace hal {

class UnitySimulatedImuDevice : public ImuDeviceInterface {
public:
    typedef net::ZMQProtobufSubscriber<IMUSample> imu_sample_sub_t;

    UnitySimulatedImuDevice(const uint32_t id, const details::property_map_t& config);
    ~UnitySimulatedImuDevice() = default;
    UnitySimulatedImuDevice(const UnitySimulatedImuDevice&) = delete;
    UnitySimulatedImuDevice(const UnitySimulatedImuDevice&&) = delete;
    UnitySimulatedImuDevice& operator=(const UnitySimulatedImuDevice&) = delete;
    UnitySimulatedImuDevice& operator=(const UnitySimulatedImuDevice&&) = delete;

    std::string deviceName() const override { return "UnitySimulatedImuDevice"; }

    uint64_t serialNumber() const override { return m_imuId; }

    bool capture(IMUSample& cameraSample) override;

private:
    zmq::context_t m_context = zmq::context_t(1);
    std::unique_ptr<imu_sample_sub_t> m_imuSampleSubscriber;
    const uint32_t m_imuId;
    int m_captureTimeoutInMilliseconds;
};
}
