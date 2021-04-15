
#include "packages/hald/include/camera_device_thread.h"
#include "packages/hal/include/device_registry.h"

#include "glog/logging.h"

using namespace hald;

CameraDeviceThread::CameraDeviceThread(const Device& deviceConfig)
    : m_context(1) {
    auto publisherAddressIterator = deviceConfig.messageproperties().data().find("publisherAddress");
    if (publisherAddressIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find publisherAddress in configuration";
        throw std::runtime_error("Unable to find publisherAddress in configuration");
    }
    const std::string& publisherAddress = publisherAddressIterator->second;

    auto topicIterator = deviceConfig.messageproperties().data().find("topic");
    if (topicIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find topic in configuration";
        throw std::runtime_error("Unable to find topic in configuration");
    }
    m_topic = topicIterator->second;

    LOG(INFO) << "Camera running on ZMQ address: " << publisherAddress << " on topic: " << m_topic;

    constexpr int highWaterMark = 1;
    constexpr int lingerPeriodInMilliseconds = 1000;
    m_cameraSamplePublisher
        = std::unique_ptr<camera_pub_t>(new camera_pub_t(m_context, publisherAddress, highWaterMark, lingerPeriodInMilliseconds));

    auto serverAddressIterator = deviceConfig.messageproperties().data().find("autoExposureServerAddress");
    if (serverAddressIterator == deviceConfig.messageproperties().data().end()) {
        LOG(ERROR) << "Unable to find autoExposureServerAddress in configuration";
        throw std::runtime_error("Unable to find autoExposureServerAddress in configuration");
    }
    const std::string& serverAddress = serverAddressIterator->second;

    m_aeRoiServer.reset(new ae_roi_server_t(m_context, serverAddress, lingerPeriodInMilliseconds, 0));

    std::map<std::string, std::string> standardMap(
        deviceConfig.deviceproperties().data().begin(), deviceConfig.deviceproperties().data().end());
    m_camera = hal::DeviceRegistry<hal::CameraDeviceInterface>::instance().create(standardMap);
    if (!m_camera.get()) {
        LOG(ERROR) << "Unable to create camera";
        throw std::runtime_error("Unable to create camera");
    }
}

CameraDeviceThread::~CameraDeviceThread() {}

void CameraDeviceThread::run() {
    while (!m_cancel) {

        hal::AutoExposureRoiCommand roiCommand;
        if (m_aeRoiServer->recv(roiCommand)) {
            hal::AutoExposureRoiResponse response;
            if (m_camera->setAutoExposureRoi(roiCommand.click_x_fraction(), roiCommand.click_y_fraction(), roiCommand.radius_fraction())) {
                response.set_disposition(hal::AERoICommandCompleted);
            } else {
                response.set_disposition(hal::AERoICommandRejected);
            }
            m_aeRoiServer->send(response);
        }

        hal::CameraSample sample;
        if (m_camera->capture(sample)) {
            if (!m_cameraSamplePublisher->send(sample, m_topic)) {
                LOG(ERROR) << "Unable to send sample";
            }
        }
    }
}