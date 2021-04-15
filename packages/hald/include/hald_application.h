#pragma once

#include "glog/logging.h"

#include "packages/hal/include/factory_registration.h"

#include "packages/hald/include/camera_device_thread.h"
#include "packages/hald/include/gps_device_thread.h"
#include "packages/hald/include/imu_device_thread.h"
#include "packages/hald/include/joystick_device_thread.h"
#include "packages/hald/include/network_health_device_thread.h"
#include "packages/hald/include/vcu_device_thread.h"
#include "packages/hald/proto/device_config.pb.h"

#include <atomic>

namespace hald {

///
/// \brief Load configuration from .conf text file
///
DeviceConfig parseConfig(const std::string& configFile);

class HaldApplication {
public:
    HaldApplication(const hald::DeviceConfig& config);

    ///
    /// \brief Clean destructor, dealing with zmq issues.
    ///
    ~HaldApplication();

    HaldApplication(const HaldApplication& other) = delete;
    HaldApplication& operator=(const HaldApplication& other) = delete;

    ///
    /// \param Signal handler
    ///
    static void signalHandler(int sig);

    ///
    /// \brief Returns if hald is still running
    ///
    bool isRunning();

private:
    struct sigaction m_newSigAction;
    std::vector<std::shared_ptr<hald::CameraDeviceThread> > m_cameras;
    std::shared_ptr<hald::GPSDeviceThread> m_gps;
    std::vector<std::shared_ptr<hald::ImuDeviceThread> > m_imus;
    std::vector<std::shared_ptr<hald::JoystickDeviceThread> > m_joysticks;
    std::shared_ptr<hald::NetworkHealthDeviceThread> m_network;
    std::shared_ptr<hald::VCUDeviceThread> m_vcu;
};
}
