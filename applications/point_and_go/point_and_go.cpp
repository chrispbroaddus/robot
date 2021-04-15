#include <fstream>

#include "packages/calibration/proto/system_calibration.pb.h"
#include "packages/core/proto/geometry.pb.h"

#include "packages/teleop/include/connection.h"
#include "packages/teleop/include/context.h"
#include "packages/teleop/proto/backend_message.pb.h"
#include "packages/teleop/proto/vehicle_message.pb.h"

#include "packages/filesystem/include/filesystem.h"
#include "packages/hald/include/camera_settings_handler.h"
#include "packages/net/include/zmq_req_client.h"
#include "packages/net/include/zmq_select.h"
#include "packages/serialization/include/proto.h"

#include "packages/estimation/estimator.h"
#include "packages/estimation/proto/estimator_options.pb.h"
#include "packages/executor/executor.h"
#include "packages/executor/proto/executor_options.pb.h"
#include "packages/executor/proto_helpers.h"
#include "packages/perception/perception.h"
#include "packages/perception/proto/perception_options.pb.h"
#include "packages/planning/planner.h"
#include "packages/planning/proto/trajectory_planner_options.pb.h"
#include "packages/planning/utils.h"

#include "applications/point_and_go/proto/point_and_go_options.pb.h"

#include "applications/point_and_go/proto/docking_options.pb.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/text_format.h"
#include "packages/docking/dock_command_executor.h"
#include "packages/perception/proto/detection.pb.h"

DEFINE_string(docking_options, "", "Docking specific options");
DEFINE_string(point_and_go_options, "", "Point-and-go specific options");
DEFINE_string(executor_options, "", "Executor specific options");
DEFINE_string(arc_planner_options, "", "Arc-planner specific options");
DEFINE_string(estimator_options, "", "Estimator-specific options");
DEFINE_string(trajectory_options, "", "Trajectory generation options");
DEFINE_string(teleop_options, "", "Teleoperation options");
DEFINE_string(camera_calibration, "", "System camera calibration");
DEFINE_string(auth_token, "", "Path to the auth token for websocket connection");
DEFINE_string(backend, "", "Override the backend URL provided in teleop options");
DEFINE_string(mode, "point-and-go", "point-and-go or docking.");

namespace perception {
class BasicPerception : public Perception {
public:
    BasicPerception(
        const PerceptionOptions& options, std::unique_ptr<TerrainRepresentation> terrain, const calibration::SystemCalibration& calibration)
        : Perception(options, std::move(terrain)) {
        CHECK(this->addDevices(calibration));
    }
};
} // perception

namespace estimation {
class DefaultEstimator : public Estimator {
public:
    DefaultEstimator(const EstimatorOptions& /*options*/) {}
};
} // estimation

namespace zippy {

using executor::Executor;
using executor::ExecutorOptions;
using executor::loadDefaultExecutorOptions;

using planning::Comms;
using planning::ZMQComms;
using planning::TrajectoryOptions;
using planning::loadDefaultTrajectoryOptions;
using planning::TrajectoryGenerator;
using planning::ScaledVelocityTrajectory;
using planning::SquareTrajectory;
using planning::SquareTrajectoryWithDeceleration;
using planning::TrajectoryPlanner;
using planning::ArcPlanner;
using planning::ArcPlannerOptions;
using planning::loadDefaultArcPlannerOptions;

using estimation::Estimator;
using estimation::DefaultEstimator;
using estimation::EstimatorOptions;
using estimation::loadDefaultEstimatorOptions;

using perception::Perception;
using perception::PerceptionOptions;
using perception::BasicPerception;

using zippy::applications::PointAndGoOptions;
using zippy::applications::DockingOptions;

PointAndGoOptions defaultPointAndGoOptions() {
    PointAndGoOptions options;
    std::fstream input("config/global/point_and_go_options.default.pbtxt", std::ios::in);
    CHECK(input.good());
    std::stringstream buffer;
    buffer << input.rdbuf();
    CHECK(google::protobuf::TextFormat::MergeFromString(buffer.str(), &options));
    return options;
}

DockingOptions defaultDockingOptions() {
    DockingOptions options;
    std::fstream input("config/global/docking_options.default.pbtxt", std::ios::in);
    CHECK(input.good());
    std::stringstream buffer;
    buffer << input.rdbuf();
    CHECK(google::protobuf::TextFormat::MergeFromString(buffer.str(), &options));
    return options;
}

///
///\brief Route object detection results to websocket
/// \return True if detection results are available and sent.
///
bool routeObjectDetectionResults(zmq::context_t& context, zmq::socket_t& frontCameraSocket, zmq::socket_t& rearCameraSocket,
    zmq::socket_t& leftCameraSocket, zmq::socket_t& rightCameraSocket, const PointAndGoOptions& options, net::ZMQSelectLoop& select,
    teleop::Connection& conn);

int main(int argc, char** argv) {
    google::InstallFailureSignalHandler();
    google::InitGoogleLogging(argv[0]);

    gflags::SetUsageMessage("Receive frames over zmq and push to websocket");
    gflags::SetVersionString("0.0.1");
    gflags::ParseCommandLineFlags(&argc, &argv, false);
    FLAGS_logtostderr = true;

    CHECK(FLAGS_mode == "point-and-go" || FLAGS_mode == "docking") << "The -mode value should be `point-and-go` or `docking`.";

    CHECK(!FLAGS_camera_calibration.empty()) << "Require system calibration";

    teleop::Context teleop_context;

    PointAndGoOptions options;
    if (!FLAGS_point_and_go_options.empty()) {
        LOG(INFO) << "Point-and-go options file: " << FLAGS_point_and_go_options;
        CHECK(planning::loadOptions(FLAGS_point_and_go_options, options));
    } else {
        LOG(INFO) << "Using default point-and-go options";
        options = defaultPointAndGoOptions();
    }
    LOG(INFO) << "PointAndGoOptions: " << options.DebugString();

    DockingOptions dockingOptions;

    if (FLAGS_mode == "docking") {
        if (!FLAGS_docking_options.empty()) {
            LOG(INFO) << "Docking option file: " << FLAGS_docking_options;
            CHECK(planning::loadOptions(FLAGS_docking_options, dockingOptions));
        } else {
            LOG(INFO) << "Using default docking option";
            dockingOptions = defaultDockingOptions();
        }
        LOG(INFO) << "DockingOptions: " << dockingOptions.DebugString();
    }

    ExecutorOptions executor_options;
    if (!FLAGS_executor_options.empty()) {
        LOG(INFO) << "Executor options file: " << FLAGS_executor_options;
        CHECK(planning::loadOptions(FLAGS_executor_options, executor_options));
    } else {
        LOG(INFO) << "Using default executor options";
        executor_options = executor::loadDefaultExecutorOptions();
    }
    LOG(INFO) << "ExecutorOptions: " << executor_options.DebugString();

    EstimatorOptions estimator_options;
    if (!FLAGS_estimator_options.empty()) {
        LOG(INFO) << "Estimator options file: " << FLAGS_estimator_options;
        CHECK(planning::loadOptions(FLAGS_estimator_options, estimator_options));
    } else {
        LOG(INFO) << "Using default estimator options";
        estimator_options = loadDefaultEstimatorOptions();
    }
    LOG(INFO) << "EstimatorOptions: " << estimator_options.DebugString();

    ArcPlannerOptions arc_planner_options;
    if (!FLAGS_arc_planner_options.empty()) {
        LOG(INFO) << "arc-planner options file: " << FLAGS_arc_planner_options;
        CHECK(planning::loadOptions(FLAGS_arc_planner_options, arc_planner_options));
    } else {
        LOG(INFO) << "Using default arc-planner options";
        arc_planner_options = loadDefaultArcPlannerOptions();
    }
    LOG(INFO) << "ArcPlannerOptions: " << arc_planner_options.DebugString();

    TrajectoryOptions trajectory_options;
    if (!FLAGS_trajectory_options.empty()) {
        LOG(INFO) << "trajectory options file: " << FLAGS_trajectory_options;
        CHECK(planning::loadOptions(FLAGS_trajectory_options, trajectory_options));
    } else {
        LOG(INFO) << "Using default trajectory options";
        trajectory_options = loadDefaultTrajectoryOptions();
    }
    LOG(INFO) << "TrajectoryOptions: " << trajectory_options.DebugString();

    // Load options for teleop
    teleop::ConnectionOptions teleop_options;
    if (!FLAGS_teleop_options.empty()) {
        LOG(INFO) << "teleop options file: " << FLAGS_teleop_options;
        CHECK(serialization::loadProtoText(FLAGS_teleop_options, &teleop_options));
    } else {
        LOG(INFO) << "using default teleop options";
        teleop_options = teleop::loadDefaultOptions();
    }

    // Load system calibration
    calibration::SystemCalibration calibration;
    CHECK(planning::loadOptions(FLAGS_camera_calibration, calibration, planning::SerializationType::JSON));

    // Start apriltag pose detector
    std::shared_ptr<docking::FiducialPoseSourceInterface> fiducialPoseSource;
    std::unique_ptr<docking::FiducialStationFinder> fiducialStationFinder;
    std::unique_ptr<docking::DockCommandExecutor> dockCommandExecutor;

    if (FLAGS_mode == "docking") {
        docking::InverseKinematicDockerOptions inverseKinematicDockerOptions;
        if (!FLAGS_inverse_kinematic_docker_options.empty()) {
            inverseKinematicDockerOptions = docking::loadInverseKinematicDockerOptions(FLAGS_inverse_kinematic_docker_options);
        } else {
            inverseKinematicDockerOptions = docking::loadDefaultInverseKinematicDockerOptions();
        }

        perception::AprilTagConfig aprilTagConfig;
        if (!FLAGS_apriltag_config.empty()) {
            aprilTagConfig = perception::loadAprilTagConfig(FLAGS_apriltag_config);
        } else {
            aprilTagConfig = perception::loadDefaultAprilTagConfig();
        }

        perception::AprilTagDetectorOptions aprilTagDetectorOptions;
        if (!FLAGS_apriltag_detector_options.empty()) {
            aprilTagDetectorOptions = perception::loadAprilTagDetectorOptions(FLAGS_apriltag_detector_options);
        } else {
            aprilTagDetectorOptions = perception::loadDefaultAprilTagDetectorOptions();
        }

        fiducialPoseSource.reset(new docking::ApriltagFiducialPoseSource(options.camera_address(), options.camera_topic(),
            calibration.cameraintrinsiccalibration(0), aprilTagConfig, aprilTagDetectorOptions));

        fiducialStationFinder.reset(new docking::FiducialStationFinder(
            fiducialPoseSource, dockingOptions.station_finder_publisher_port(), 1, 1000, 0, dockingOptions.station_calibration_file()));

        dockCommandExecutor.reset(new docking::DockCommandExecutor(inverseKinematicDockerOptions, fiducialPoseSource,
            dockingOptions.station_finder_publisher_port(), dockingOptions.vehicle_calibration_file()));
    }

    // Allow overriding of backend from command line
    if (!FLAGS_backend.empty()) {
        teleop_options.set_backend_address(FLAGS_backend);
    }

    std::unique_ptr<TrajectoryGenerator> generator(new SquareTrajectoryWithDeceleration(trajectory_options));
    std::unique_ptr<TrajectoryGenerator> scaledGenerator(
        new ScaledVelocityTrajectory(std::move(generator), options.velocity_scaling_horizon()));
    std::shared_ptr<TrajectoryPlanner> trajectoryPlanner(new ArcPlanner(arc_planner_options, std::move(scaledGenerator)));
    using planning::StateMachineAdapter;
    std::unique_ptr<StateMachineAdapter> plannerWrapper;
    plannerWrapper.reset(new StateMachineAdapter(trajectoryPlanner));

    using perception::FlatTerrain;
    std::unique_ptr<FlatTerrain> terrain(new FlatTerrain);
    std::unique_ptr<Perception> perception_ptr(new BasicPerception(PerceptionOptions(), std::move(terrain), calibration));
    std::unique_ptr<Comms> comms_ptr(
        new ZMQComms(executor_options.address(), executor_options.port(), executor_options.require_acknowledge()));

    std::unique_ptr<Estimator> estimator(new DefaultEstimator(estimator_options));
    std::shared_ptr<Executor> executor;
    executor.reset(
        new Executor(executor_options, std::move(estimator), std::move(plannerWrapper), std::move(perception_ptr), std::move(comms_ptr)));

    // Set up camera sources for teleop
    if (!populateCalibrationParameters(&teleop_options, calibration)) {
        LOG(FATAL) << "failed to populate teleop options from system calibration";
    }

    // Open the connection to the backend
    teleop::Connection conn(teleop_options);
    std::error_code err = conn.Dial();
    if (err) {
        LOG(FATAL) << "failed to connect to backend:" << err;
        exit(EXIT_FAILURE);
    }

    // Set up callbacks
    conn.OnJoystick([&executor](const teleop::JoystickCommand& cmd) {
        LOG(INFO) << "router received a joystick command: " << cmd.linearvelocity() << ", " << cmd.curvature();
        executor->execute(cmd);
    });

    if (FLAGS_mode == "point-and-go") // While in the docking mode, point-and-go command is not processed.
    {
        conn.OnPointAndGo([&executor](const teleop::PointAndGoCommand& cmd) {
            LOG(INFO) << "router received a point-and-go command: " << cmd.imagex() << ", " << cmd.imagey();
            LOG(INFO) << cmd.DebugString();
            executor->execute(cmd);
        });

        conn.OnStopRequested([&executor](const teleop::StopCommand& cmd) {
            LOG(INFO) << "router received a stop command: " << cmd.DebugString();
            executor->execute(cmd);
        });
    }

    if (FLAGS_mode == "docking") {
        conn.OnDockingRequested([&executor, &dockCommandExecutor](const teleop::DockCommand& cmd) {
            LOG(INFO) << "router received a docking command: " << cmd.station_id();
            dockCommandExecutor->execute(executor, cmd);
        });
    }

    zmq::context_t context = zmq::context_t(1);

    hald::CameraSettingsHandler cameraSettingsHandler(context, teleop_options);

    bool camera_available = false;
    bool gps_available = false;
    bool docking_observation_available = false;
    bool docking_status_available = false;

    net::ZMQSelectLoop select;
    zmq::socket_t frames(context, ZMQ_SUB);
    frames.setsockopt(ZMQ_RCVHWM, 1);
    zmq::socket_t gps(context, ZMQ_SUB);
    gps.setsockopt(ZMQ_RCVHWM, 1);
    try {
        auto last_upload = std::chrono::steady_clock::now();
        std::chrono::seconds interval(options.thumbnail_interval());

        // subscribe to the camera socket
        frames.connect(options.camera_address());
        frames.setsockopt(ZMQ_SUBSCRIBE, options.camera_topic().data(), options.camera_topic().size());
        select.OnProtobuf<hal::CameraSample>(frames, "camera", [&](const hal::CameraSample& s) {
            // only upload frames once every N seconds
            auto now = std::chrono::steady_clock::now();
            if (now - last_upload < interval) {
                return;
            }

            LOG(INFO) << "uploading a thumbnail";
            last_upload = now;
            conn.SendStillImage(s);
        });
        camera_available = true;
    } catch (zmq::error_t& error) {
        LOG(ERROR) << "Camera unavailable";
    }

    try {
        // subscribe to the GPS socket
        gps.connect(options.gps_address());
        gps.setsockopt(ZMQ_SUBSCRIBE, options.gps_topic().data(), options.gps_topic().size());
        select.OnProtobuf<hal::GPSTelemetry>(gps, "gps", [&](const hal::GPSTelemetry& t) { conn.Send(t); });
        gps_available = true;
    } catch (zmq::error_t& error) {
        LOG(ERROR) << "GPS unavailable";
    }

    if (FLAGS_mode == "point-and-go") // While in the docking mode, point-and-go command is not processed.
    {
        conn.OnExposure([&cameraSettingsHandler](const teleop::ExposureCommand& cmd) {
            LOG(INFO) << "router received an exposure command with camera id: " << cmd.camera_id() << ", centerX: " << cmd.centerx()
                      << ", centerY: " << cmd.centery() << ", radius: " << cmd.radius();
            cameraSettingsHandler.setAutoExposureRoI(cmd);
        });
        conn.OnResetExposure([&cameraSettingsHandler](const teleop::ResetExposureCommand& cmd) {
            LOG(INFO) << "router received a reset exposure command with camera id: " << cmd.camera_id();
            LOG(INFO) << cmd.DebugString();
            cameraSettingsHandler.resetAutoExposureRoI(cmd);
        });
    }

    if (FLAGS_mode == "docking") {
        zmq::socket_t docking_observations(context, ZMQ_SUB);
        docking_observations.setsockopt(ZMQ_RCVHWM, 1);
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            // subscribe to the camera socket
            std::string dockingObservationAddr("tcp://localhost:" + dockingOptions.station_finder_publisher_port());
            docking_observations.connect(dockingObservationAddr);
            VLOG(2) << __PRETTY_FUNCTION__ << "... dockingObservationAddr : " << dockingObservationAddr;
            docking_observations.setsockopt(
                ZMQ_SUBSCRIBE, dockingOptions.docking_observation_topic().data(), dockingOptions.docking_observation_topic().size());
            select.OnProtobuf<docking::DockingStationList>(
                docking_observations, "docking_observation", [&](const docking::DockingStationList& s) {
                    VLOG(2) << __PRETTY_FUNCTION__ << "... docking observation is received from docking.cpp";
                    teleop::DockingObservation obs;
                    for (int i = 0; i < s.docking_stations_size(); i++) {
                        obs.add_station_ids(s.docking_stations(i).station_id());
                    }
                    conn.Send(obs);
                });
            docking_observation_available = true;
        } catch (zmq::error_t& error) {
            LOG(ERROR) << "Docking observation unavailable";
        }

        zmq::socket_t dockingStatus(context, ZMQ_SUB);
        dockingStatus.setsockopt(ZMQ_RCVHWM, 1);
        try {
            // subscribe to the GPS socket
            dockingStatus.connect(dockingOptions.docking_status_address());
            dockingStatus.setsockopt(
                ZMQ_SUBSCRIBE, dockingOptions.docking_status_topic().data(), dockingOptions.docking_status_topic().size());
            select.OnProtobuf<teleop::DockingStatus>(
                dockingStatus, "docking_status", [&](const teleop::DockingStatus& t) { conn.Send(t); });
            docking_status_available = true;

        } catch (zmq::error_t& error) {
            LOG(ERROR) << "Docking status unavailable";
        }
    }

    // Route object detection results
    zmq::socket_t frontCameraDetSocket(context, ZMQ_SUB);
    zmq::socket_t rearCameraDetSocket(context, ZMQ_SUB);
    zmq::socket_t leftCameraDetSocket(context, ZMQ_SUB);
    zmq::socket_t rightCameraDetSocket(context, ZMQ_SUB);
    frontCameraDetSocket.setsockopt(ZMQ_RCVHWM, 1);
    rearCameraDetSocket.setsockopt(ZMQ_RCVHWM, 1);
    leftCameraDetSocket.setsockopt(ZMQ_RCVHWM, 1);
    rightCameraDetSocket.setsockopt(ZMQ_RCVHWM, 1);

    bool detection_available = routeObjectDetectionResults(
        context, frontCameraDetSocket, rearCameraDetSocket, leftCameraDetSocket, rightCameraDetSocket, options, select, conn);

    select.OnTick([&]() {
        // service webrtc messages
        teleop_context.ProcessMessages(std::chrono::milliseconds(10));
    });

    if (camera_available || gps_available || docking_observation_available || docking_status_available || detection_available) {
        // listen for frames and GPS and send them to the backend
        select.Loop();
    } else {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            teleop_context.ProcessMessages(std::chrono::milliseconds(10));
        }
    }
    return EXIT_SUCCESS;
}

bool routeObjectDetectionResults(zmq::context_t& /*context*/, zmq::socket_t& frontCameraSocket, zmq::socket_t& rearCameraSocket,
    zmq::socket_t& leftCameraSocket, zmq::socket_t& rightCameraSocket, const PointAndGoOptions& options, net::ZMQSelectLoop& select,
    teleop::Connection& conn) {

    std::vector<zmq::socket_t*> sockets = { { &frontCameraSocket, &rearCameraSocket, &leftCameraSocket, &rightCameraSocket } };
    std::vector<std::string> addresses = { { options.front_camera_detection_2d_address(), options.rear_camera_detection_2d_address(),
        options.left_camera_detection_2d_address(), options.right_camera_detection_2d_address() } };
    std::vector<std::string> topics = { { options.front_camera_detection_2d_topic(), options.rear_camera_detection_2d_topic(),
        options.left_camera_detection_2d_topic(), options.right_camera_detection_2d_topic() } };

    bool existMessageToSend = false;

    constexpr int numCameras = 4;
    for (int i = 0; i < numCameras; i++) {
        if (addresses[i].empty()) {
            continue;
        }
        try {
            // subscribe to the GPS socket
            sockets[i]->connect(addresses[i]);
            sockets[i]->setsockopt(ZMQ_SUBSCRIBE, topics[i].data(), topics[i].size());
            select.OnProtobuf<perception::Detection>(*sockets[i], topics[i], [&](const perception::Detection& t) {
                VLOG(2) << __PRETTY_FUNCTION__ << " ... routing detection, # of bbox : " << t.box_detection().bounding_boxes_size();
                conn.Send(t.box_detection());
            });
            existMessageToSend = true;
        } catch (zmq::error_t& error) {
            VLOG(2) << __PRETTY_FUNCTION__ << "Detection result are not available from the address : " << addresses[i]
                    << ", topic : " << topics[i];
        }
    }
    return existMessageToSend;
}

} // zippy

int main(int argc, char** argv) { return zippy::main(argc, argv); }
