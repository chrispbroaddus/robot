load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "coordinate_frame_id_utils",
    hdrs = [
        "coordinate_frame_id_utils.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hal",
        "//packages/hal/proto:coordinate_frame_id",
    ],
)

cc_binary(
    name = "enumerate_dc1394_cameras",
    srcs = ["enumerate_dc1394_cameras.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hal",
    ],
)

cc_binary(
    name = "udp_socket_joystick_control",
    srcs = ["udp_socket_joystick_control.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hal",
    ],
)

cc_binary(
    name = "dummy_telemetry_publisher",
    srcs = ["dummy_telemetry_publisher.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hal",
    ],
)

cc_binary(
    name = "dummy_telemetry_subscriber",
    srcs = ["dummy_telemetry_subscriber.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hal",
    ],
)
