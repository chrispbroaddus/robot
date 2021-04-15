cc_library(
    name = "imu_streamer",
    srcs = [
        "src/imu_publisher.cpp",
        "src/imu_publisher.h",
        "src/imu_streamer.cpp",
    ],
    hdrs = [
        "include/imu_reading.h",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//packages/hal",
        "//packages/hal/proto:imu_sample",
        "//packages/net",
        "//packages/unity_plugins/utils",
    ],
)

cc_binary(
    name = "test_imu_sample_subscriber",
    srcs = [
        "src/test_imu_sample_subscriber.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "//external:glog",
        "//packages/hal",
        "//packages/hal/proto:imu_sample",
        "//packages/net",
    ],
)
