cc_test(
    name = "test",
    srcs = [
        "vcu_telemetry_publisher_test.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "//external:gflags",
        "//packages/net",
        "//packages/unity_plugins/vcu_telemetry_publisher",
        "@gtest//:main",
    ],
)

cc_test(
    name = "network_test",
    srcs = [
        "vcu_telemetry_publisher_network_test.cpp",
    ],
    tags = [
        "network_test",
    ],
    deps = [
        "//external:gflags",
        "//packages/net",
        "//packages/unity_plugins/vcu_telemetry_publisher",
        "@gtest//:main",
    ],
)

cc_binary(
    name = "test_vcu_telemetry_subscriber",
    srcs = [
        "test_vcu_telemetry_subscriber.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "//external:glog",
        "//packages/hal",
        "//packages/net",
    ],
)
