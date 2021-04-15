#include "packages/hald/include/hald_application.h"

#include "glog/logging.h"

#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#include <fstream>
#include <sys/stat.h>
#include <syslog.h>

using namespace hald;

namespace {
const std::string s_daemonName = "zippy-hald";
std::atomic_bool s_stop(false);
}

DeviceConfig hald::parseConfig(const std::string& configFile) {
    std::ifstream inputFileStream(configFile, std::ios::in);
    if (!inputFileStream.is_open()) {
        throw std::runtime_error("hald unable to open config file");
    }

    google::protobuf::io::IstreamInputStream inputStream(&inputFileStream);

    hald::DeviceConfig config;
    if (!google::protobuf::TextFormat::Parse(&inputStream, &config)) {
        throw std::runtime_error("hald failed to parse config file");
    }
    return config;
}

HaldApplication::HaldApplication(const hald::DeviceConfig& config) {

    m_newSigAction.sa_handler = signalHandler;
    sigemptyset(&m_newSigAction.sa_mask);
    m_newSigAction.sa_flags = 0;
    sigaction(SIGHUP, &m_newSigAction, NULL);
    sigaction(SIGTERM, &m_newSigAction, NULL);
    sigaction(SIGINT, &m_newSigAction, NULL);

    hal::Initialize();

    for (int i = 0; i < config.camera_size(); i++) {
        const hald::Device& device = config.camera(i);
        auto camera = std::make_shared<hald::CameraDeviceThread>(device);
        m_cameras.push_back(camera);

        if (camera.get()) {
            camera->start();
        }
    }

    for (int i = 0; i < config.imu_size(); i++) {
        const hald::Device& device = config.imu(i);
        auto imu = std::make_shared<hald::ImuDeviceThread>(device);
        m_imus.push_back(imu);

        if (imu.get()) {
            imu->start();
        }
    }

    for (int i = 0; i < config.joystick_size(); i++) {
        const hald::Device& device = config.joystick(i);
        auto joystick = std::make_shared<hald::JoystickDeviceThread>(device);
        m_joysticks.push_back(joystick);

        if (joystick.get()) {
            joystick->start();
        }
    }

    if (config.has_vcu()) {
        const hald::Device& device = config.vcu();
        m_vcu = std::make_shared<hald::VCUDeviceThread>(device);

        if (m_vcu.get()) {
            m_vcu->start();
        }
    }

    if (config.has_gps()) {
        const hald::Device& device = config.gps();
        m_gps = std::make_shared<hald::GPSDeviceThread>(device);

        if (m_gps.get()) {
            m_gps->start();
        }
    }

    if (config.has_network()) {
        const hald::Device& device = config.network();
        m_network = std::make_shared<hald::NetworkHealthDeviceThread>(device);
        if (m_network) {
            m_network->start();
        }
    }
}

HaldApplication::~HaldApplication() {

    for (auto& camera : m_cameras) {
        camera->stop();
    }

    for (auto& imu : m_imus) {
        imu->stop();
    }

    for (auto& joystick : m_joysticks) {
        joystick->stop();
    }

    if (m_network) {
        m_network->stop();
    }

    if (m_gps) {
        m_gps->stop();
    }

    if (m_vcu) {
        m_vcu->stop();
    }
}

///
/// \param Signal handler
///
void HaldApplication::signalHandler(int sig) {
    switch (sig) {
    case SIGHUP:
        syslog(LOG_WARNING, "SIGHUP");
        break;
    case SIGINT:
        syslog(LOG_INFO, "SIGINT");
        s_stop = true;
        break;
    case SIGTERM:
        syslog(LOG_INFO, "SIGTERM");
        s_stop = true;
        break;
    default:
        syslog(LOG_WARNING, "Unhandled signal %s", strsignal(sig));
        break;
    }
}

///
/// \brief Returns if hald is still running
bool HaldApplication::isRunning() { return !s_stop; }
