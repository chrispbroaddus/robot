#pragma once

#include "applications/mercury/proto/connections.pb.h"
#include <glog/logging.h>

namespace unity_plugins {

inline mercury::Connection* addConnection(mercury::Connections& connections, mercury::topic topic, const std::string& address, int port) {
    auto connection = connections.add_connection();
    connection->set_topic(topic);
    connection->set_address(address);
    connection->set_port(port);

    return connection;
}

inline mercury::Connections defaultSimulatorConnections() {
    mercury::Connections connections;

    addConnection(connections, mercury::topic::front_left_stereo, "127.0.0.1", 5501);
    addConnection(connections, mercury::topic::front_left_stereo_depth, "127.0.0.1", 5502);
    addConnection(connections, mercury::topic::front_left_stereo_pointcloud, "127.0.0.1", 5503);

    addConnection(connections, mercury::topic::front_right_stereo, "127.0.0.1", 5504);
    addConnection(connections, mercury::topic::front_right_stereo_depth, "127.0.0.1", 5505);
    addConnection(connections, mercury::topic::front_right_stereo_pointcloud, "127.0.0.1", 5506);

    addConnection(connections, mercury::topic::rear_left_stereo, "127.0.0.1", 5507);
    addConnection(connections, mercury::topic::rear_left_stereo_depth, "127.0.0.1", 5508);
    addConnection(connections, mercury::topic::rear_left_stereo_pointcloud, "127.0.0.1", 5509);

    addConnection(connections, mercury::topic::rear_right_stereo, "127.0.0.1", 5510);
    addConnection(connections, mercury::topic::rear_right_stereo_depth, "127.0.0.1", 5511);
    addConnection(connections, mercury::topic::rear_right_stereo_pointcloud, "127.0.0.1", 5512);

    addConnection(connections, mercury::topic::front_fisheye, "127.0.0.1", 5513);
    addConnection(connections, mercury::topic::front_fisheye_depth, "127.0.0.1", 5514);
    addConnection(connections, mercury::topic::front_fisheye_pointcloud, "127.0.0.1", 5515);

    addConnection(connections, mercury::topic::rear_fisheye, "127.0.0.1", 5516);
    addConnection(connections, mercury::topic::rear_fisheye_depth, "127.0.0.1", 5517);
    addConnection(connections, mercury::topic::rear_fisheye_pointcloud, "127.0.0.1", 5518);

    addConnection(connections, mercury::topic::left_fisheye, "127.0.0.1", 5519);
    addConnection(connections, mercury::topic::left_fisheye_depth, "127.0.0.1", 5520);
    addConnection(connections, mercury::topic::left_fisheye_pointcloud, "127.0.0.1", 5521);

    addConnection(connections, mercury::topic::right_fisheye, "127.0.0.1", 5522);
    addConnection(connections, mercury::topic::right_fisheye_depth, "127.0.0.1", 5523);
    addConnection(connections, mercury::topic::right_fisheye_pointcloud, "127.0.0.1", 5524);

    addConnection(connections, mercury::topic::simulator_commands, "127.0.0.1", 5401);

    addConnection(connections, mercury::topic::telemetry, "127.0.0.1", 5804);
    addConnection(connections, mercury::topic::trajectory, "127.0.0.1", 5803);

    addConnection(connections, mercury::topic::gps, "127.0.0.1", 5700);
    addConnection(connections, mercury::topic::imu, "127.0.0.1", 5701);
    addConnection(connections, mercury::topic::fiducial_poses, "127.0.0.1", 7601);
    addConnection(connections, mercury::topic::joystick, "127.0.0.1", 5702);
    addConnection(connections, mercury::topic::ground_truth_telemetry, "127.0.0.1", 7501);
    addConnection(connections, mercury::topic::simulator_stats, "127.0.0.1", 6001);

    return connections;
}
}
