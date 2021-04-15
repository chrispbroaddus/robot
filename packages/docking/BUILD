load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "docking_status_publisher",
    srcs = [
        "docking_status_publisher.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//packages/net",
        "//packages/teleop/proto:vehicle_message",
    ],
)

cc_library(
    name = "fiducial_pose_source",
    srcs = [
        "apriltag_fiducial_pose_source.cpp",
        "ground_truth_fiducial_pose_source.cpp",
    ],
    hdrs = [
        "apriltag_fiducial_pose_source.h",
        "fiducial_pose_source_interface.h",
        "ground_truth_fiducial_pose_source.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:apriltags",
        "//external:eigen",
        "//packages/core",
        "//packages/docking/proto:docking_station",
        "//packages/estimation",
        "//packages/math",
        "//packages/net",
        "//packages/perception",
        "//packages/perception/fiducials",
        "//packages/perception/fiducials/proto:fiducial_poses",
        "//thirdparty/Sophus",
    ],
)

cc_library(
    name = "station_finder",
    srcs = [
        "fiducial_station_finder.cpp",
    ],
    hdrs = [
        "fiducial_station_finder.h",
        "station_finder_interface.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":fiducial_pose_source",
        "//external:apriltags",
        "//packages/docking/proto:station_search_envelope",
        "//packages/net",
        "//packages/planning:utils",
    ],
)

cc_library(
    name = "vcu_telemetry_subscriber",
    srcs = [
        "vcu_telemetry_subscriber.cpp",
    ],
    hdrs = [
        "vcu_ik_telemetry_state.h",
        "vcu_telemetry_subscriber.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//packages/core",
        "//packages/estimation",
        "//packages/hal/proto:vcu_telemetry",
        "//packages/net",
        "//packages/perception",
    ],
)

cc_library(
    name = "inverse_kinematic_controller",
    srcs = [
        "inverse_kinematic_controller.cpp",
    ],
    hdrs = [
        "inverse_kinematic_controller.h",
        "inverse_kinematic_controller_helper.h",
        "vcu_ik_telemetry_state.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:eigen",
        "//external:glog",
        "//packages/core/proto:geometry",
        "//packages/core/proto:timestamp",
        "//packages/docking/proto:docking_station",
        "//packages/docking/proto:vehicle_calibration",
        "//packages/hal/proto:vcu_commands",
        "//packages/math",
        "//packages/net",
        "//packages/perception/fiducials/proto:fiducial_poses",
        "//thirdparty/Sophus",
    ],
)

cc_library(
    name = "inverse_kinematic_docker",
    srcs = [
        "inverse_kinematic_docker.cpp",
    ],
    hdrs = [
        "inverse_kinematic_docker.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "docking_status_publisher",
        "fiducial_pose_source",
        "inverse_kinematic_controller",
        "vcu_telemetry_subscriber",
        "//packages/core",
        "//packages/docking:station_finder",
        "//packages/docking/proto:inverse_kinematic_docker_options",
        "//packages/docking/proto:station_search_envelope",
        "//packages/estimation",
        "//packages/executor:execution",
        "//packages/math/geometry",
        "//packages/perception",
        "//packages/teleop/proto:vehicle_message",
    ],
)

cc_library(
    name = "dock_command_executor",
    srcs = [
        "dock_command_executor.cpp",
    ],
    hdrs = [
        "dock_command_executor.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "inverse_kinematic_docker",
        "//external:eigen",
        "//external:glog",
        "//packages/net",
        "//packages/teleop/proto:backend_message",
    ],
)
