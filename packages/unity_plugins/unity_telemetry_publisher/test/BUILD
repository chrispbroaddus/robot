cc_test(
    name = "test",
    srcs = [
        "unity_telemetry_publisher_test.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "//external:gflags",
        "//packages/core/test:common",
        "//packages/net",
        "//packages/unity_plugins/unity_telemetry_publisher",
        "@gtest//:main",
    ],
)

cc_test(
    name = "network_test",
    srcs = [
        "unity_telemetry_publisher_network_test.cpp",
    ],
    tags = [
        "network_test",
    ],
    deps = [
        "//external:gflags",
        "//packages/core/test:common",
        "//packages/net",
        "//packages/unity_plugins/unity_telemetry_publisher",
        "@gtest//:main",
    ],
)

cc_binary(
    name = "test_unity_telemetry_subscriber",
    srcs = [
        "test_unity_telemetry_subscriber.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "//external:glog",
        "//packages/hal",
        "//packages/net",
        "//packages/unity_plugins/proto:unity_telemetry_envelope",
        "//packages/unity_plugins/unity_telemetry_publisher",
    ],
)
