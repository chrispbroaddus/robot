#include "packages/hal/include/factory_registration.h"

#include "packages/hal/include/drivers/cameras/dc1394/dc1394_driver_factory.h"
#include "packages/hal/include/drivers/cameras/depth/realsense_driver_factory.h"
#include "packages/hal/include/drivers/cameras/flycapture/flycapture_driver_factory.h"
#include "packages/hal/include/drivers/cameras/playback/playback_camera_device_factory.h"
#include "packages/hal/include/drivers/cameras/unity_simulated/unity_simulated_camera_device_factory.h"

#include "packages/hal/include/drivers/gps/gpsd/gpsd_device_factory.h"

#include "packages/hal/include/drivers/imus/unity_simulated/unity_simulated_imu_device_factory.h"

#include "packages/hal/include/drivers/joysticks/sdl2/joystick_device_factory.h"

#include "packages/hal/include/drivers/network_health/network_health_driver_factory.h"

#include "packages/hal/include/drivers/vcu/udp_socket/udp_socket_vcu_driver_factory.h"
#include "packages/hal/include/drivers/vcu/unity_simulated/unity_simulated_vcu_factory.h"

namespace hal {

void FactoryRegistration::registerFactories() {
    static DC1394DriverFactory s_dc1394DriverFactory;
    static FlycaptureDriverFactory s_flycaptureDriverFactory;
    static PlaybackCameraDeviceFactory s_playbackCameraDeviceFactory;
    static RealsenseDriverFactory s_realsenseDriverFactory;
    static UnitySimulatedCameraDeviceFactory s_unitySimulatedCameraDeviceFactory;

    static GPSDDeviceFactory s_gpsdDeviceFactory;

    static UnitySimulatedImuDeviceFactory s_unitySimulatedImuDeviceFactory;

    static JoystickDeviceFactory s_joystickDeviceFactory;

    static NetworkHealthDriverFactory s_networkHealthFactory;

    static vcu::UdpSocketVcuDriverFactory s_udpSocketVcuDriverFactory;
    static vcu::UnitySimulatedVcuFactory s_unitySimulatedVcuFactory;
}
} // hal
