#include "packages/hal/include/device_registry.h"
#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"
#include "packages/hal/include/drivers/cameras/dc1394/dc1394_driver.h"
#include "packages/hal/include/drivers/network_health/network_health_driver_interface.h"
#include "packages/hal/include/factory_registration.h"
#include "packages/hal/include/string_utils.h"
#include "packages/net/include/zmq_rep_server.h"
#include "packages/net/include/zmq_topic_pub.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

#include <thread>

using namespace hal;

TEST(HAL, DeviceRegistryUnitySimulatedCamera) {

    hal::Initialize();

    hal::details::property_map_t deviceConfig;
    deviceConfig[hal::details::FACTORY_NAME_KEY] = "UnitySimulatedCamera";
    deviceConfig["cameraId"] = "FrontLeftStereo";
    deviceConfig["publisherAddress"] = "tcp://localhost:5501";
    deviceConfig["topic"] = "camera";
    deviceConfig["subscriberHighWaterMark"] = std::to_string(1);
    deviceConfig["captureTimeoutInMilliseconds"] = std::to_string(1000);

    EXPECT_NO_THROW(std::shared_ptr<CameraDeviceInterface> camera = DeviceRegistry<CameraDeviceInterface>::instance().create(deviceConfig));
}

TEST(HAL, DeviceRegistryDc1394Camera) {

    hal::Initialize();

    dc1394_t* bus;
    dc1394error_t err;
    dc1394camera_list_t* list;
    bus = dc1394_new();
    if (!bus) {
        LOG(ERROR) << "Dc1394: Bus creation failed";
        throw std::runtime_error("Dc1394: Bus creation failed");
    }
    err = dc1394_camera_enumerate(bus, &list);
    if (err != DC1394_SUCCESS) {
        LOG(ERROR) << "Dc1394: Camera enumeration failed";
        throw std::runtime_error("DC1394: Camera enumeration failed");
    }
    if (list->num == 0) {
        LOG(ERROR) << "Dc1394: No cameras found";
        throw std::runtime_error("No cameras found");
    }
    uint64_t guid = list->ids[0].guid;

    hal::details::property_map_t deviceConfig;
    deviceConfig[hal::details::FACTORY_NAME_KEY] = "Dc1394Camera";
    deviceConfig["guid"] = std::to_string(guid);
    deviceConfig["mode"] = "FORMAT7_0";
    deviceConfig["color"] = "MONO8";
    deviceConfig["width"] = std::to_string(2440);
    deviceConfig["height"] = std::to_string(2046);
    deviceConfig["left"] = std::to_string(0);
    deviceConfig["top"] = std::to_string(0);
    deviceConfig["nDma"] = std::to_string(4);
    deviceConfig["fps"] = std::to_string(60);
    deviceConfig["externalTrigger"] = "OFF";

    EXPECT_NO_THROW(std::shared_ptr<CameraDeviceInterface> camera = DeviceRegistry<CameraDeviceInterface>::instance().create(deviceConfig));
}

TEST(HAL, DeviceRegistryNetworkHealth) {
    hal::Initialize();

    hal::details::property_map_t deviceConfig;
    deviceConfig[hal::details::FACTORY_NAME_KEY] = "NetworkHealth";
    deviceConfig["google.com"] = "";
    EXPECT_NO_THROW(DeviceRegistry<NetworkHealthDriverInterface>::instance().create(deviceConfig));
}
