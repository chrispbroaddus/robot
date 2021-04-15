#include "packages/hald/include/camera_settings_handler.h"

namespace hald {

hal::CameraId getCameraIdFromString(const std::string& cameraId) {
    hal::CameraId id;
    if (cameraId == "FrontLeftStereo") {
        id = hal::CameraId::FrontLeftStereo;
    } else if (cameraId == "FrontRightStereo") {
        id = hal::CameraId::FrontRightStereo;
    } else if (cameraId == "RearLeftStereo") {
        id = hal::CameraId::RearLeftStereo;
    } else if (cameraId == "RearRightStereo") {
        id = hal::CameraId::RearRightStereo;
    } else if (cameraId == "FrontFisheye") {
        id = hal::CameraId::FrontFisheye;
    } else if (cameraId == "RearFisheye") {
        id = hal::CameraId::RearFisheye;
    } else if (cameraId == "LeftFisheye") {
        id = hal::CameraId::LeftFisheye;
    } else if (cameraId == "RightFisheye") {
        id = hal::CameraId::RightFisheye;
    } else {
        throw std::runtime_error("Invalid Camera Id");
    }
    return id;
}

CameraSettingsHandler::CameraSettingsHandler(zmq::context_t& context, const teleop::ConnectionOptions& connectionOptions) {
    for (int i = 0; i < connectionOptions.video_sources_size(); i++) {
        m_AERoIClientMap[connectionOptions.video_sources(i).camera().role()].reset(
            new ae_roi_client_t(context, connectionOptions.video_sources(i).settings_server_address(), 1000, 1000));
    }
}

bool CameraSettingsHandler::setAutoExposureRoI(const teleop::ExposureCommand& cmd) {
    hal::AutoExposureRoiCommand roiCommand;
    hal::AutoExposureRoiResponse roiResponse;

    roiCommand.set_click_x_fraction(cmd.centerx());
    roiCommand.set_click_y_fraction(cmd.centery());
    roiCommand.set_radius_fraction(cmd.radius());

    if (!m_AERoIClientMap[getCameraIdFromString(cmd.camera_id())]->send(roiCommand)) {
        return false;
    }

    if (!m_AERoIClientMap[getCameraIdFromString(cmd.camera_id())]->recv(roiResponse)) {
        return false;
    }

    return roiResponse.disposition() == hal::AERoICommandCompleted;
}

bool CameraSettingsHandler::resetAutoExposureRoI(const teleop::ResetExposureCommand& cmd) {
    hal::AutoExposureRoiCommand roiCommand;
    hal::AutoExposureRoiResponse roiResponse;

    roiCommand.set_click_x_fraction(0);
    roiCommand.set_click_y_fraction(0);
    roiCommand.set_radius_fraction(0);

    if (!m_AERoIClientMap[getCameraIdFromString(cmd.camera_id())]->send(roiCommand)) {
        return false;
    }

    if (!m_AERoIClientMap[getCameraIdFromString(cmd.camera_id())]->recv(roiResponse)) {
        return false;
    }

    return roiResponse.disposition() == hal::AERoICommandCompleted;
}
}
