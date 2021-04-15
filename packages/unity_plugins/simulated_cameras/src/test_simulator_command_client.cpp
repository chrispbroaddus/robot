#include "glog/logging.h"
#include "packages/hal/proto/simulator_command_envelope.pb.h"
#include "packages/hal/proto/simulator_command_response.pb.h"
#include "packages/hal/proto/simulator_reset_command.pb.h"
#include "packages/net/include/zmq_req_client.h"

#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <zmq.hpp>

std::unique_ptr<net::ZMQProtobufReqClient<hal::SimulatorCommandEnvelope, hal::SimulatorCommandResponse> > s_client;

void sendReset(bool robot, bool world, bool streams) {
    hal::SimulatorCommandEnvelope commandMsg;
    auto resetCommand = new hal::SimulatorResetCommand();
    resetCommand->set_resetworld(world);
    resetCommand->set_resetrobot(robot);
    resetCommand->set_resetstreams(streams);

    commandMsg.set_allocated_resetcommand(resetCommand);

    LOG(INFO) << "CAMERA CONTROL CLIENT : Sending Reset message.";

    s_client->send(commandMsg);
}

bool resetMenu() {
    bool world = false;
    bool robot = false;
    bool streams = false;

    while (1) {
        std::cout << "Configure Reset Message" << std::endl;
        std::cout << "- Robot (r) = " << (robot ? "true" : "false") << std::endl;
        std::cout << "- World (w) = " << (world ? "true" : "false") << std::endl;
        std::cout << "- Streams (s) = " << (streams ? "true" : "false") << std::endl;
        std::cout << "- Send (t)" << std::endl;
        std::cout << "- Cancel (c)" << std::endl;
        char option;

        std::cin >> option;

        switch (option) {
        case 'r':
            robot = !robot;
            break;
        case 'w':
            world = !world;
            break;
        case 's':
            streams = !streams;
            break;
        case 't':
            sendReset(robot, world, streams);
            return true;
        case 'c':
            return false;
        default:
            LOG(INFO) << "Invalid option";
        }
    }
}

void sendStats(bool enable) {
    hal::SimulatorCommandEnvelope commandMsg;
    auto statsCommand = new hal::SimulatorStatsCommand();
    statsCommand->set_enable(enable);

    commandMsg.set_allocated_statscommand(statsCommand);

    LOG(INFO) << "CAMERA CONTROL CLIENT : Sending Stats message.";

    s_client->send(commandMsg);
}

bool statsMenu() {

    bool enable = false;

    while (1) {
        std::cout << "Configure Stats Message" << std::endl;
        std::cout << "- Enable (e) = " << (enable ? "true" : "false") << std::endl;
        std::cout << "- Send (t)" << std::endl;
        std::cout << "- Cancel (c)" << std::endl;
        char option;

        std::cin >> option;

        switch (option) {
        case 'e':
            enable = !enable;
            break;
        case 't':
            sendStats(enable);
            return true;
        case 'c':
            return false;
        default:
            LOG(INFO) << "Invalid option";
        }
    }
}

void sendCameraConfig(hal::CameraId cameraId, bool enable, bool depth, bool pointcloud) {
    hal::SimulatorCommandEnvelope commandMsg;
    auto command = new hal::SimulatorCameraCommand();
    command->set_cameraid(cameraId);
    command->set_enable(enable);
    command->set_enabledepth(depth);
    command->set_enablexyz(pointcloud);

    commandMsg.set_allocated_cameracommand(command);

    LOG(INFO) << "CAMERA CONTROL CLIENT : Sending Camera message.";

    s_client->send(commandMsg);
}

bool cameraMenu() {
    hal::CameraId cameraId = hal::CameraId_MIN;
    bool enable = false;
    bool depth = false;
    bool pointcloud = false;
    char option;
    int id;

    while (1) {
        std::cout << "Configure Camera Message" << std::endl;
        std::cout << "- CameraId (i) = " << hal::CameraId_Name(cameraId) << std::endl;
        std::cout << "- Enable (e) = " << (enable ? "true" : "false") << std::endl;
        std::cout << "- Enable Depth (d) = " << (depth ? "true" : "false") << std::endl;
        std::cout << "- Enable Pointcloud (p) = " << (pointcloud ? "true" : "false") << std::endl;
        std::cout << "- Send (t)" << std::endl;
        std::cout << "- Cancel (c)" << std::endl;

        std::cin >> option;

        switch (option) {
        case 'i':
            id = (int)cameraId + 1;
            if (id > (int)hal::CameraId_MAX) {
                id = (int)hal::CameraId_MIN;
            }
            cameraId = (hal::CameraId)id;
            break;
        case 'e':
            enable = !enable;
            break;
        case 'd':
            depth = !depth;
            break;
        case 'p':
            pointcloud = !pointcloud;
            break;
        case 't':
            sendCameraConfig(cameraId, enable, depth, pointcloud);
            return true;
        case 'c':
            return false;
        default:
            LOG(INFO) << "Invalid option";
        }
    }
}

void printCameraOutput(const std::string& name, const hal::SimulatorCameraOutput& output) {
    LOG(INFO) << "Output: " << name;
    LOG(INFO) << "Address: " << output.address();
    LOG(INFO) << "Topic: " << output.topic();
}

void handleResponse() {
    hal::SimulatorCommandResponse response;
    s_client->recv(response);

    LOG(INFO) << "CAMERA CONTROL CLIENT : Received a message, size : " << response.ByteSize();
    LOG(INFO) << "Response.status() : " << response.status();

    if (response.has_resetresponse()) {
        LOG(INFO) << "Reset response";
    } else if (response.has_cameraresponse()) {
        LOG(INFO) << "Camera response";
        printCameraOutput("image", response.cameraresponse().image());
        printCameraOutput("depth", response.cameraresponse().depth());
        printCameraOutput("pointcloud", response.cameraresponse().xyz());
    } else if (response.has_statsresponse()) {
        LOG(INFO) << "Stats response";
        LOG(INFO) << "Address: " << response.statsresponse().address();
        LOG(INFO) << "Topic: " << response.statsresponse().topic();
    }

    std::cout << "-------------------------------" << std::endl;
}

void MainMenu() {
    while (1) {
        std::cout << "Select message to send:" << std::endl;
        std::cout << "- Camera Configuration (c)" << std::endl;
        std::cout << "- Reset (r)" << std::endl;
        std::cout << "- Stats (s)" << std::endl;
        std::cout << "- Quit App (q)" << std::endl;
        char option;
        std::cin >> option;

        switch (option) {
        case 'c':
            if (cameraMenu()) {
                handleResponse();
            }
            break;
        case 'r':
            if (resetMenu()) {
                handleResponse();
            }
            break;
        case 's':
            if (statsMenu()) {
                handleResponse();
            }
            break;
        case 'q':
            return;
        default:
            LOG(INFO) << "Invalid option";
        }
    }
}

///
/// @brief Run the simulator command client for testing purposes
///
int main(int argc, char** argv) {

    google::InitGoogleLogging(argv[0]);

    if (argc != 3) {
        LOG(ERROR) << "USAGE: " << argv[0] << " IP_ADDRESS PORT_NUMBER";
        return 1;
    }

    std::string ipAddress(argv[1]);
    int configServerPort = atoi(argv[2]);

    //  Prepare our context and socket
    zmq::context_t context(1);
    std::string addr = "tcp://" + ipAddress + ":" + std::to_string(configServerPort);
    s_client.reset(new net::ZMQProtobufReqClient<hal::SimulatorCommandEnvelope, hal::SimulatorCommandResponse>(context, addr, 1000, 1000));

    MainMenu();

    auto ptr = s_client.release();
    delete ptr;

    return 0;
}
