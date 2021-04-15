#pragma once

#include "packages/data_logger/proto/config.pb.h"
#include "packages/filter_graph/include/source_filter.h"

#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/hal/proto/gps_telemetry.pb.h"
#include "packages/hal/proto/imu_sample.pb.h"
#include "packages/hal/proto/joystick_sample.pb.h"
#include "packages/hal/proto/network_health_telemetry.pb.h"
#include "packages/hal/proto/vcu_telemetry_envelope.pb.h"
#include "packages/net/include/zmq_select.h"
#include "packages/net/include/zmq_topic_sub.h"

namespace data_logger {

class DeviceSourceFilter : public filter_graph::SourceFilter {
public:
    DeviceSourceFilter(const DataLoggerConfig& config);
    ~DeviceSourceFilter();
    DeviceSourceFilter(const DeviceSourceFilter&) = delete;
    DeviceSourceFilter(const DeviceSourceFilter&&) = delete;
    DeviceSourceFilter& operator=(const DeviceSourceFilter&) = delete;
    DeviceSourceFilter& operator=(const DeviceSourceFilter&&) = delete;

    void create() override;
    void stop();
    const std::vector<std::string>& cameraStreamIds() const { return m_cameraStreamIds; }
    const std::vector<std::string>& gpsStreamIds() const { return m_gpsStreamIds; }
    const std::vector<std::string>& imuStreamIds() const { return m_imuStreamIds; }
    const std::vector<std::string>& joystickStreamIds() const { return m_joystickStreamIds; }
    const std::vector<std::string>& vcuTelemetryStreamIds() const { return m_vcuTelemetryStreamIds; }
    const std::vector<std::string>& networkHealthStreamIds() const { return m_networkHealthStreamIds; }

private:
    zmq::context_t m_context = zmq::context_t(1);
    net::ZMQSelectLoop m_select;
    std::map<std::string, std::shared_ptr<zmq::socket_t> > m_cameraSubscriber;
    std::map<std::string, std::shared_ptr<zmq::socket_t> > m_gpsSubscriber;
    std::map<std::string, std::shared_ptr<zmq::socket_t> > m_imuSubscriber;
    std::map<std::string, std::shared_ptr<zmq::socket_t> > m_joystickSubscriber;
    std::map<std::string, std::shared_ptr<zmq::socket_t> > m_networkHealthSubscriber;
    std::map<std::string, std::shared_ptr<zmq::socket_t> > m_vcuTelemetrySubscriber;

    std::vector<std::string> m_cameraStreamIds;
    std::vector<std::string> m_gpsStreamIds;
    std::vector<std::string> m_imuStreamIds;
    std::vector<std::string> m_joystickStreamIds;
    std::vector<std::string> m_networkHealthStreamIds;
    std::vector<std::string> m_vcuTelemetryStreamIds;
};
}
