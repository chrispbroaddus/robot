cc_library(
    name = "unity_telemetry_publisher",
    srcs = [
        "src/unity_telemetry_publisher.cpp",
        "src/unity_telemetry_publisher_interface.cpp",
    ],
    hdrs = [
        "include/unity_telemetry_publisher.h",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/core",
        "//packages/hal",
        "//packages/net",
        "//packages/unity_plugins/proto:unity_telemetry_envelope",
        "//packages/unity_plugins/utils",
    ],
)
