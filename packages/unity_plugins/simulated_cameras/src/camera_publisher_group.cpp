#include "packages/unity_plugins/simulated_cameras/include/camera_publisher_group.h"
#include "glog/logging.h"

using namespace unity_plugins;

CameraPublisherGroup::CameraPublisherGroup(std::shared_ptr<zmq::context_t> context,
    const std::map<hal::CameraId, CameraOutputAddresses>& cameraAddresses, int zmqLingerTimeInMilliSeconds, int zmqHighWaterMarkValue) {

    for (auto& element : cameraAddresses) {
        hal::CameraId cameraId = element.first;
        const CameraOutputAddresses& addresses = element.second;
        m_publishers[cameraId]
            = std::unique_ptr<CameraPublisher>(new CameraPublisher(context, addresses, zmqLingerTimeInMilliSeconds, zmqHighWaterMarkValue));
    }
}

void CameraPublisherGroup::sendImage(const CameraImage& cameraImage) { m_publishers.at(cameraImage.cameraId)->sendImage(cameraImage); }
