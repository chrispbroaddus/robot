load("//tools:cpp_compile_flags.bzl", "COPTS")

COMMON_COPTS = COPTS

cc_library(
    name = "proto_helpers",
    srcs = ["proto_helpers.cpp"],
    hdrs = ["proto_helpers.h"],
    deps = [
        "//external:glog",
        "//packages/estimation/proto:estimator_options",
        "//packages/planning:proto_helpers",
    ],
)

cc_library(
    name = "estimation",
    srcs = [
        "estimator.cpp",
    ],
    hdrs = [
        "estimator.h",
        "thirdparty/eigen_utils.h",
    ],
    copts = COMMON_COPTS,
    data = [
        "//:global_configuration",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":proto_helpers",
        "//external:eigen",
        "//external:glog",
        "//external:wykobi",
        "//packages/calibration/proto:coordinate_transformation",
        "//packages/core",
        "//packages/core/proto:geometry",
        "//packages/core/proto:timestamp",
        "//packages/estimation/proto:estimator_options",
        "//packages/estimation/proto:state",
        "//packages/hal/proto:device",
        "//packages/hal/proto:vcu_telemetry",
        "//packages/net",
        "//packages/planning:utils",
        "//packages/unity_plugins/proto:ground_truth_vehicle_pose",
        "//packages/unity_plugins/proto:unity_telemetry_envelope",
        "//thirdparty/Sophus",
    ],
)

cc_library(
    name = "visualization",
    srcs = ["visualization.h"],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":estimation",
        "//packages/planning:utils",
    ],
)

cc_binary(
    name = "odometry_log_writer",
    srcs = [
        "bin/odometry_log_writer.cpp",
    ],
    copts = COMMON_COPTS,
    deps = [
        ":estimation",
        "//packages/hal",
        "//packages/net",
        "//packages/planning:logging",
    ],
)

cc_binary(
    name = "ground_truth_log_writer",
    srcs = [
        "bin/ground_truth_log_writer.cpp",
    ],
    copts = COMMON_COPTS,
    deps = [
        ":estimation",
        "//packages/hal",
        "//packages/net",
        "//packages/planning:logging",
    ],
)

cc_binary(
    name = "odometry_log_reader",
    srcs = [
        "bin/odometry_log_reader.cpp",
    ],
    copts = COMMON_COPTS,
    deps = [
        ":estimation",
        "//external:gflags",
        "//packages/hal",
        "//packages/net",
        "//packages/planning:logging",
    ],
)

cc_binary(
    name = "ground_truth_estimator",
    srcs = [
        "bin/ground_truth_estimator.cpp",
    ],
    copts = COMMON_COPTS,
    deps = [
        ":estimation",
        "//external:gflags",
        "//packages/hal",
        "//packages/net",
        "//packages/planning:logging",
    ],
)

cc_binary(
    name = "visualize_pose",
    srcs = [
        "bin/visualize_pose.cpp",
    ],
    copts = ["-DHAVE_OPENGL"] + COMMON_COPTS,
    data = ["//packages/perception:bin_shaders"],
    deps = [
        ":estimation",
        ":visualization",
        "//external:SDL",
        "//external:glog",
        "//packages/perception",
        "//packages/perception:trackball",
        "//packages/perception:visualization",
        "//packages/planning:planner",
        "//packages/planning:utils",
    ],
)

cc_binary(
    name = "wheel_odometry_estimator",
    srcs = [
        "bin/wheel_odometry_estimator.cpp",
    ],
    copts = COMMON_COPTS,
    deps = [
        ":estimation",
        "//external:SDL",
        "//external:glog",
        "//packages/planning:planner",
        "//packages/planning:utils",
    ],
)

#Helper tool to dump default proto options to `.pbtxt` files
cc_binary(
    name = "write_options",
    srcs = ["bin/write_options.cpp"],
    copts = COMMON_COPTS,
    deps = [
        ":estimation",
    ],
)
