#include <chrono>
#include <string>

#include <zmq.hpp>

#include "gflags/gflags.h"
#include "glog/logging.h"

#include "packages/hal/proto/camera_sample.pb.h"
#include "packages/hal/proto/gps_telemetry.pb.h"

#include "packages/net/include/zmq_select.h"

#include "packages/streamer/proto/stream.pb.h"
#include "packages/teleop/include/connection.h"
#include "packages/teleop/include/context.h"
#include "packages/teleop/proto/backend_message.pb.h"
#include "packages/teleop/proto/connection_options.pb.h"
#include "packages/teleop/proto/vehicle_message.pb.h"

DEFINE_string(camera_name, "front", "Name for the camera device");
DEFINE_string(camera_addr, "tcp://localhost:15556", "ZMQ socket for camera publisher, in format 'tcp://host:port'");
DEFINE_string(camera_topic, "camera", "topic for camera publisher");
DEFINE_string(gps_addr, "tcp://localhost:15557", "ZMQ socket for gps publisher, in format 'tcp://host:port'");
DEFINE_string(gps_topic, "gps", "topic for gps publisher");
DEFINE_string(backend, "ws://mission-control.zippy.ai", "URL for backend, in format 'ws://host:port'");
DEFINE_string(vehicle_id, "r01", "Vehicle ID for connection to backend");
DEFINE_string(auth_token, "", "Authentication token for connection to backend");

int main(int argc, char** argv) {
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    gflags::SetUsageMessage("Receive frames over zmq and push to websocket");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);

    FLAGS_logtostderr = true;

    teleop::Context ctx;

    // Create the connection object
    teleop::ConnectionOptions opts;
    opts.set_backend_address(FLAGS_backend);
    opts.set_vehicle_id(FLAGS_vehicle_id);
    opts.set_auth_token(FLAGS_auth_token);
    opts.set_jpeg_quality(80);
    opts.mutable_webrtc()->set_min_udp_port(52000);
    opts.mutable_webrtc()->set_max_udp_port(53000);

    teleop::VideoSource* video = opts.add_video_sources();
    video->mutable_camera()->mutable_device()->set_name(FLAGS_camera_name);
    video->mutable_source()->set_address(FLAGS_camera_addr);
    video->mutable_source()->set_topic(FLAGS_camera_topic);
    video->mutable_source()->set_output_width(640);
    video->mutable_source()->set_output_height(360);

    teleop::Connection conn(opts);

    // Add one camera

    // Open the websocket connection to the backend
    std::error_code err = conn.Dial();
    if (err) {
        LOG(FATAL) << "failed to connect to backend:" << err;
    }

    conn.OnJoystick([](const teleop::JoystickCommand& cmd) {
        LOG(INFO) << "router received a joystick command: " << cmd.linearvelocity() << ", " << cmd.curvature();
    });

    conn.OnPointAndGo([](const teleop::PointAndGoCommand& cmd) {
        LOG(INFO) << "router received a point-and-go command: " << cmd.imagex() << ", " << cmd.imagey();
    });

    zmq::context_t context = zmq::context_t(1);

    // subscribe to the camera socket
    zmq::socket_t frames(context, ZMQ_SUB);
    frames.setsockopt(ZMQ_RCVHWM, 1);
    frames.connect(FLAGS_camera_addr);
    frames.setsockopt(ZMQ_SUBSCRIBE, FLAGS_camera_topic.data(), FLAGS_camera_topic.size());

    // subscribe to the GPS socket
    zmq::socket_t gps(context, ZMQ_SUB);
    gps.setsockopt(ZMQ_RCVHWM, 1);
    gps.connect(FLAGS_gps_addr);
    gps.setsockopt(ZMQ_SUBSCRIBE, FLAGS_gps_topic.data(), FLAGS_gps_topic.size());

    // listen for frames and GPS and send them to the backend
    net::ZMQSelectLoop select;

    select.OnProtobuf<hal::GPSTelemetry>(gps, "gps", [&](const hal::GPSTelemetry& t) {
        // send GPS sample to backend
        conn.Send(t);
    });

    select.OnTick([&]() {
        // process incoming messages on the webrtc thread.
        ctx.ProcessMessages(std::chrono::milliseconds(10));
    });

    LOG(INFO) << "setup done, entering select loop...";
    select.Loop();

    return 0;
}
