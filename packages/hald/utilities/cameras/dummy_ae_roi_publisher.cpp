#include "packages/hald/include/camera_settings_handler.h"

int main() {

    zmq::context_t context = zmq::context_t(1);
    teleop::ConnectionOptions connectionOptions;

    connectionOptions.add_video_sources();
    connectionOptions.mutable_video_sources(0)->mutable_camera()->set_role(hal::FrontFisheye);
    connectionOptions.mutable_video_sources(0)->set_settings_server_address("tcp://localhost:5254");

    hald::CameraSettingsHandler cameraSettingsHandler(context, connectionOptions);

    while (true) {
        std::string cameraId;
        float floatVal;
        teleop::ExposureCommand command;
        std::cout << "Enter cameraId(string):\n";
        std::cin >> cameraId;
        command.set_camera_id(cameraId);
        std::cout << "Enter centerx:\n";
        std::cin >> floatVal;
        command.set_centerx(floatVal);
        std::cout << "Enter centery:\n";
        std::cin >> floatVal;
        command.set_centery(floatVal);
        std::cout << "Enter radius:\n";
        std::cin >> floatVal;
        command.set_radius(floatVal);
        cameraSettingsHandler.setAutoExposureRoI(command);
    }
}
