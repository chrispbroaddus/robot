load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "docking_status_publisher_test",
    srcs = [
        "docking_status_publisher_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//external:eigen",
        "//external:glog",
        "//packages/core/test:common",
        "//packages/docking:docking_status_publisher",
        "@gtest//:main",
    ],
)

cc_test(
    name = "inverse_kinematic_controller_helper_test",
    srcs = [
        "inverse_kinematic_controller_helper_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//external:glog",
        "//packages/docking:inverse_kinematic_controller",
        "//thirdparty/Sophus",
        "@gtest//:main",
    ],
)

cc_library(
    name = "synthetic_fiducial_pose_publisher",
    srcs = [
        "synthetic_fiducial_pose_publisher.cpp",
    ],
    hdrs = [
        "synthetic_fiducial_pose_publisher.h",
    ],
    copts = COPTS,
    deps = [
        "//packages/calibration/proto:coordinate_transformation",
        "//packages/hal",
        "//packages/net",
        "//packages/perception/fiducials/proto:fiducial_poses",
    ],
)

cc_test(
    name = "fiducial_pose_source_test",
    srcs = [
        "fiducial_pose_source_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "synthetic_fiducial_pose_publisher",
        "//external:glog",
        "//packages/core/test:common",
        "//packages/docking:fiducial_pose_source",
        "//thirdparty/Sophus",
        "@gtest//:main",
    ],
)

cc_library(
    name = "synthetic_vcu_telemetry_publisher",
    srcs = [
        "synthetic_vcu_telemetry_publisher.cpp",
    ],
    hdrs = [
        "synthetic_vcu_telemetry_publisher.h",
    ],
    copts = COPTS,
    deps = [
        "//packages/calibration/proto:coordinate_transformation",
        "//packages/hal",
        "//packages/net",
        "//packages/perception/fiducials/proto:fiducial_poses",
    ],
)

cc_test(
    name = "vcu_telemetry_subscriber_test",
    srcs = [
        "vcu_telemetry_subscriber_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "synthetic_vcu_telemetry_publisher",
        "//external:glog",
        "//packages/core/test:common",
        "//packages/docking:vcu_telemetry_subscriber",
        "@gtest//:main",
    ],
)

cc_test(
    name = "inverse_kinematic_docker_test",
    srcs = [
        "inverse_kinematic_docker_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//packages/core/test:common",
        "//packages/docking:inverse_kinematic_docker",
        "@gtest//:main",
    ],
)

cc_binary(
    name = "test_docking_station_list_subscriber",
    srcs = [
        "test_docking_station_list_subscriber.cpp",
    ],
    copts = COPTS,
    deps = [
        "//external:glog",
        "//packages/docking/proto:docking_station",
        "//packages/net",
    ],
)

cc_binary(
    name = "inverse_kinematic_docker_options_serailize_test",
    srcs = [
        "inverse_kinematic_docker_options_serailize_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//packages/docking/proto:inverse_kinematic_docker_options",
    ],
)
