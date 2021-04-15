cc_library(
    name = "vcu_telemetry_publisher",
    srcs = [
        "src/vcu_telemetry_publisher.cpp",
        "src/vcu_telemetry_publisher_interface.cpp",
    ],
    hdrs = [
        "include/vcu_telemetry_publisher.h",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/core",
        "//packages/hal",
        "//packages/net",
        "//packages/unity_plugins/utils",
    ],
)
