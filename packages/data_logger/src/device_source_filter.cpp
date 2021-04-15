
#include "packages/data_logger/include/device_source_filter.h"
#include "packages/data_logger/include/data_logger_sample.h"
#include "packages/data_logger/proto/config.pb.h"

#include <algorithm>
#include <iostream>

using namespace hal;

namespace data_logger {

DeviceSourceFilter::DeviceSourceFilter(const DataLoggerConfig& config)
    : filter_graph::SourceFilter("DeviceSourceFilter", 100) {

    for (int i = 0; i < config.camera_size(); i++) {
        const data_logger::Stream& stream = config.camera(i);

        const std::string& name = stream.name();
        const std::string& serverAddress = stream.server_address();
        const std::string& topic = stream.topic();

        if (m_cameraSubscriber.find(name) != m_cameraSubscriber.end()) {
            throw std::runtime_error("Stream name already exists: " + name);
        }

        m_cameraSubscriber[name] = std::make_shared<zmq::socket_t>(m_context, ZMQ_SUB);
        m_cameraSubscriber[name]->setsockopt(ZMQ_RCVHWM, 100);
        m_cameraSubscriber[name]->connect(serverAddress);
        m_cameraSubscriber[name]->setsockopt(ZMQ_SUBSCRIBE, topic.data(), topic.size());
        m_select.OnProtobuf<hal::CameraSample>(*m_cameraSubscriber[name], topic, [name, this](const hal::CameraSample& cameraSampleData) {
            auto container = std::make_shared<filter_graph::Container>();
            auto cameraSample = std::make_shared<data_logger::details::DataloggerSample<hal::CameraSample> >(name);

            cameraSample->data() = cameraSampleData;
            LOG(INFO) << "Creating image sample: " << name;
            container->add(cameraSample->streamId(), cameraSample);

            getOutputQueue()->enqueue(container);
            send();
        });
        m_cameraStreamIds.push_back(name);
    }

    for (int i = 0; i < config.joystick_size(); i++) {
        const data_logger::Stream& stream = config.joystick(i);

        const std::string& name = stream.name();
        const std::string& serverAddress = stream.server_address();
        const std::string& topic = stream.topic();

        if (m_joystickSubscriber.find(name) != m_joystickSubscriber.end()) {
            throw std::runtime_error("Stream name already exists: " + name);
        }

        m_joystickSubscriber[name] = std::make_shared<zmq::socket_t>(m_context, ZMQ_SUB);
        m_joystickSubscriber[name]->setsockopt(ZMQ_RCVHWM, 100);
        m_joystickSubscriber[name]->connect(serverAddress);
        m_joystickSubscriber[name]->setsockopt(ZMQ_SUBSCRIBE, topic.data(), topic.size());
        m_select.OnProtobuf<hal::JoystickSample>(
            *m_joystickSubscriber[name], topic, [name, this](const hal::JoystickSample& joystickSampleData) {
                auto container = std::make_shared<filter_graph::Container>();
                auto joystickSample = std::make_shared<data_logger::details::DataloggerSample<hal::JoystickSample> >(name);

                joystickSample->data() = joystickSampleData;
                LOG(INFO) << "Creating joystick sample: " << name;
                container->add(joystickSample->streamId(), joystickSample);

                getOutputQueue()->enqueue(container);
                send();
            });
        m_joystickStreamIds.push_back(name);
    }

    for (int i = 0; i < config.vcu_telemetry_size(); i++) {
        const data_logger::Stream& stream = config.vcu_telemetry(i);

        const std::string& name = stream.name();
        const std::string& serverAddress = stream.server_address();
        const std::string& topic = stream.topic();

        if (m_vcuTelemetrySubscriber.find(name) != m_vcuTelemetrySubscriber.end()) {
            throw std::runtime_error("Stream name already exists: " + name);
        }

        m_vcuTelemetrySubscriber[name] = std::make_shared<zmq::socket_t>(m_context, ZMQ_SUB);
        m_vcuTelemetrySubscriber[name]->setsockopt(ZMQ_RCVHWM, 100);
        m_vcuTelemetrySubscriber[name]->connect(serverAddress);
        m_vcuTelemetrySubscriber[name]->setsockopt(ZMQ_SUBSCRIBE, topic.data(), topic.size());
        m_select.OnProtobuf<hal::VCUTelemetryEnvelope>(
            *m_vcuTelemetrySubscriber[name], topic, [name, this](const hal::VCUTelemetryEnvelope& vcuTelemetryData) {
                auto container = std::make_shared<filter_graph::Container>();
                auto telemetrySample = std::make_shared<data_logger::details::DataloggerSample<hal::VCUTelemetryEnvelope> >(name);

                telemetrySample->data() = vcuTelemetryData;
                LOG(INFO) << "Creating telemetry sample: " << name;
                container->add(telemetrySample->streamId(), telemetrySample);

                getOutputQueue()->enqueue(container);
                send();
            });
        m_vcuTelemetryStreamIds.push_back(name);
    }

    for (int i = 0; i < config.gps_size(); i++) {
        const data_logger::Stream& stream = config.gps(i);

        const std::string& name = stream.name();
        const std::string& serverAddress = stream.server_address();
        const std::string& topic = stream.topic();

        if (m_gpsSubscriber.find(name) != m_gpsSubscriber.end()) {
            throw std::runtime_error("Stream name already exists: " + name);
        }

        m_gpsSubscriber[name] = std::make_shared<zmq::socket_t>(m_context, ZMQ_SUB);
        m_gpsSubscriber[name]->setsockopt(ZMQ_RCVHWM, 100);
        m_gpsSubscriber[name]->connect(serverAddress);
        m_gpsSubscriber[name]->setsockopt(ZMQ_SUBSCRIBE, topic.data(), topic.size());
        m_select.OnProtobuf<hal::GPSTelemetry>(*m_gpsSubscriber[name], topic, [name, this](const hal::GPSTelemetry& gpsTelemetryData) {
            auto container = std::make_shared<filter_graph::Container>();
            auto gpsSample = std::make_shared<data_logger::details::DataloggerSample<hal::GPSTelemetry> >(name);

            gpsSample->data() = gpsTelemetryData;
            LOG(INFO) << "Creating gps sample: " << name;
            container->add(gpsSample->streamId(), gpsSample);

            getOutputQueue()->enqueue(container);
            send();
        });
        m_gpsStreamIds.push_back(name);
    }

    for (int i = 0; i < config.imu_size(); i++) {
        const data_logger::Stream& stream = config.imu(i);

        const std::string& name = stream.name();
        const std::string& serverAddress = stream.server_address();
        const std::string& topic = stream.topic();

        if (m_imuSubscriber.find(name) != m_imuSubscriber.end()) {
            throw std::runtime_error("Stream name already exists: " + name);
        }

        m_imuSubscriber[name] = std::make_shared<zmq::socket_t>(m_context, ZMQ_SUB);
        m_imuSubscriber[name]->setsockopt(ZMQ_RCVHWM, 100);
        m_imuSubscriber[name]->connect(serverAddress);
        m_imuSubscriber[name]->setsockopt(ZMQ_SUBSCRIBE, topic.data(), topic.size());
        m_select.OnProtobuf<hal::IMUSample>(*m_imuSubscriber[name], topic, [name, this](const hal::IMUSample& imuSampleData) {
            auto container = std::make_shared<filter_graph::Container>();
            auto imuSample = std::make_shared<data_logger::details::DataloggerSample<hal::IMUSample> >(name);

            imuSample->data() = imuSampleData;
            LOG(INFO) << "Creating imu sample: " << name;
            container->add(imuSample->streamId(), imuSample);

            getOutputQueue()->enqueue(container);
            send();
        });
        m_imuStreamIds.push_back(name);
    }

    for (int i = 0; i < config.network_health_size(); i++) {
        const data_logger::Stream& stream = config.network_health(i);

        const std::string& name = stream.name();
        const std::string& serverAddress = stream.server_address();
        const std::string& topic = stream.topic();

        if (m_networkHealthSubscriber.find(name) != m_networkHealthSubscriber.end()) {
            throw std::runtime_error("Stream name already exists: " + name);
        }

        m_networkHealthSubscriber[name] = std::make_shared<zmq::socket_t>(m_context, ZMQ_SUB);
        m_networkHealthSubscriber[name]->setsockopt(ZMQ_RCVHWM, 100);
        m_networkHealthSubscriber[name]->connect(serverAddress);
        m_networkHealthSubscriber[name]->setsockopt(ZMQ_SUBSCRIBE, topic.data(), topic.size());
        m_select.OnProtobuf<hal::NetworkHealthTelemetry>(
            *m_networkHealthSubscriber[name], topic, [name, this](const hal::NetworkHealthTelemetry& networkHealthData) {
                auto container = std::make_shared<filter_graph::Container>();
                auto networkHealth = std::make_shared<data_logger::details::DataloggerSample<hal::NetworkHealthTelemetry> >(name);

                networkHealth->data() = networkHealthData;
                LOG(INFO) << "Creating network health sample: " << name;
                container->add(networkHealth->streamId(), networkHealth);

                getOutputQueue()->enqueue(container);
                send();
            });
        m_networkHealthStreamIds.push_back(name);
    }
}

DeviceSourceFilter::~DeviceSourceFilter() {}

void DeviceSourceFilter::create() { m_select.Loop(); }

void DeviceSourceFilter::stop() { m_select.StopLoop(); }
}
