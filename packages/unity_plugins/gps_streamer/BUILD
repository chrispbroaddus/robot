cc_library(
    name = "gps_streamer",
    srcs = [
        "src/gps_publisher.cpp",
        "src/gps_publisher.h",
        "src/gps_streamer.cpp",
    ],
    hdrs = [
        "include/gps_reading.h",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//packages/hal",
        "//packages/hal/proto:gps_telemetry",
        "//packages/net",
        "//packages/unity_plugins/utils",
    ],
)

cc_binary(
    name = "test_gps_telemetry_subscriber",
    srcs = [
        "src/test_gps_telemetry_subscriber.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "//external:glog",
        "//packages/hal",
        "//packages/hal/proto:gps_telemetry",
        "//packages/net",
    ],
)
