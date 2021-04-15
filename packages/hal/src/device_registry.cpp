
#include "packages/hal/include/device_registry.h"
#include "packages/hal/include/device_factory.h"
#include "packages/hal/include/drivers/cameras/camera_device_interface.h"
#include "packages/hal/include/drivers/gps/gps_device_interface.h"
#include "packages/hal/include/drivers/imus/imu_device_interface.h"
#include "packages/hal/include/drivers/joysticks/joystick_device_interface.h"
#include "packages/hal/include/drivers/network_health/network_health_driver_interface.h"
#include "packages/hal/include/drivers/vcu/vcu_device_interface.h"

namespace hal {

template <typename DEVICE_CLASS_T>
std::shared_ptr<DEVICE_CLASS_T> DeviceRegistry<DEVICE_CLASS_T>::create(const details::property_map_t& config) {

    const std::string& factoryName = config.find(hal::details::FACTORY_NAME_KEY)->second;

    typename std::map<std::string, DeviceFactory<DEVICE_CLASS_T>*>::iterator it = m_factories.find(factoryName);

    if (it == m_factories.end()) {
        LOG(ERROR) << "Unable to locate factory name: " << factoryName;
        return std::shared_ptr<DEVICE_CLASS_T>();
    }

    std::shared_ptr<DEVICE_CLASS_T> deviceInstance = it->second->create(config);
    if (deviceInstance.get() == 0) {
        LOG(ERROR) << "Factory unable to create: " << factoryName;
        return deviceInstance;
    }

    return deviceInstance;
}

template class hal::DeviceRegistry<hal::CameraDeviceInterface>;
template class hal::DeviceRegistry<hal::GPSDeviceInterface>;
template class hal::DeviceRegistry<hal::ImuDeviceInterface>;
template class hal::DeviceRegistry<hal::JoystickDeviceInterface>;
template class hal::DeviceRegistry<hal::NetworkHealthDriverInterface>;
template class hal::DeviceRegistry<hal::vcu::VCUDeviceInterface>;

} // hal
