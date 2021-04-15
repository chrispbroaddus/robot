#include "packages/unity_plugins/simulator_settings_reader/include/simulator_settings_reader.h"
#include "glog/logging.h"
#include "google/protobuf/util/json_util.h"
#include "packages/unity_simulator/proto/simulator_settings.pb.h"
#include <exception>
#include <fstream>
#include <set>
#include <sstream>

using namespace unity_plugins;

SimulatorSettingsReader::SimulatorSettingsReader(const std::string& filename) {
    std::ifstream jsonStream = std::ifstream(filename);
    if (!jsonStream.is_open()) {
        throw std::runtime_error("The json stream data is invalid.");
    }

    std::stringstream buffer;
    buffer << jsonStream.rdbuf();
    auto status = google::protobuf::util::JsonStringToMessage(buffer.str(), &m_settings);

    if (!status.ok()) {
        throw std::runtime_error("The settings file contains invalid data: " + status.ToString());
    }

    // vehicle
    m_interopSettings.vehicle.modelId = m_settings.vehicle().model_id();
    auto& vehicleCalibration = m_settings.vehicle().vehicle_calibration();
    m_interopSettings.vehicle.vehicleCalibration.center.x = vehicleCalibration.center().x();
    m_interopSettings.vehicle.vehicleCalibration.center.y = vehicleCalibration.center().y();

    m_interopSettings.vehicle.vehicleCalibration.frontLeft.x = vehicleCalibration.frontleft().x();
    m_interopSettings.vehicle.vehicleCalibration.frontLeft.y = vehicleCalibration.frontleft().y();

    m_interopSettings.vehicle.vehicleCalibration.frontRight.x = vehicleCalibration.frontright().x();
    m_interopSettings.vehicle.vehicleCalibration.frontRight.y = vehicleCalibration.frontright().y();

    m_interopSettings.vehicle.vehicleCalibration.middleLeft.x = vehicleCalibration.middleleft().x();
    m_interopSettings.vehicle.vehicleCalibration.middleLeft.y = vehicleCalibration.middleleft().y();

    m_interopSettings.vehicle.vehicleCalibration.middleRight.x = vehicleCalibration.middleright().x();
    m_interopSettings.vehicle.vehicleCalibration.middleRight.y = vehicleCalibration.middleright().y();

    m_interopSettings.vehicle.vehicleCalibration.rearLeft.x = vehicleCalibration.rearleft().x();
    m_interopSettings.vehicle.vehicleCalibration.rearLeft.y = vehicleCalibration.rearleft().y();

    m_interopSettings.vehicle.vehicleCalibration.rearRight.x = vehicleCalibration.rearright().x();
    m_interopSettings.vehicle.vehicleCalibration.rearRight.y = vehicleCalibration.rearright().y();

    // application
    m_interopSettings.application.targetFrameRate = m_settings.application().targetframerate();
    m_interopSettings.application.targetFixedDeltaTime = m_settings.application().targetfixeddeltatime();

    // cameras
    m_interopSettings.cameras.frameDownloadBufferSize = m_settings.cameras().framedownloadbuffersize();
    m_interopSettings.cameras.cameraStartDepth = m_settings.cameras().camerastartdepth();
    m_interopSettings.cameras.maxTextureSize = m_settings.cameras().maxtexturesize();

    if (m_settings.cameras().camera().size() != hal::CameraId_ARRAYSIZE) {
        throw std::runtime_error(
            "Wrong number of camera entries. Must have " + std::to_string(hal::CameraId_ARRAYSIZE) + " unique cameras");
    }

    std::set<hal::CameraId> idCheck;

    for (int ii = 0; ii < m_settings.cameras().camera().size(); ++ii) {
        auto& camIn = m_settings.cameras().camera(ii);
        auto& camOut = m_interopSettings.cameras.camera[ii];

        if (!idCheck.insert(camIn.cameraid()).second) {
            throw new std::runtime_error(
                "Duplicate camera settings found. Cameras must be unique: " + hal::CameraId_Name(camIn.cameraid()));
        }

        camOut.cameraId = camIn.cameraid();
        camOut.greyscale = camIn.greyscale();
        camOut.nearClippingPlaneDistanceMeters = camIn.nearclippingplanedistancemeters();
        camOut.farClippingPlaneDistanceMeters = camIn.farclippingplanedistancemeters();
        camOut.distortedMeshRowFineness = camIn.distortedmeshrowfineness();
        camOut.distortedMeshColFineness = camIn.distortedmeshcolfineness();
        camOut.calibratedPlaneScaleFactor = camIn.calibratedplanescalefactor();
        camOut.maxDepthMapDistanceMeters = camIn.maxdepthmapdistancemeters();
        camOut.imageScale = camIn.imagescale();
        camOut.imageZoom = camIn.imagezoom();
        camOut.enabled = camIn.enabled();
        camOut.depthEnabled = camIn.depthenabled();
        camOut.pointcloudEnabled = camIn.xyzenabled();
    }

    // networking
    m_interopSettings.networking.zmqSendReceiveTimeoutMs = m_settings.networking().zmqsendreceivetimeoutms();
    m_interopSettings.networking.zmqLingerTimeMs = m_settings.networking().zmqlingertimems();
    m_interopSettings.networking.zmqHighWaterMark = m_settings.networking().zmqhighwatermark();

    // ground truth
    m_interopSettings.groundTruth.publishVehiclePosition = m_settings.groundtruth().publishvehicleposition();
    m_interopSettings.groundTruth.publishFiducialPoses = m_settings.groundtruth().publishfiducialposes();

    // scene settings
    if (m_settings.scene().sceneurl().length() > 2048) {
        throw std::runtime_error("Scene URL is too long. Must be 2048 characters or less");
    }
    strcpy(m_interopSettings.scene.sceneUrl, m_settings.scene().sceneurl().c_str());
}
