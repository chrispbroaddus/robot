cc_test(
    name = "test",
    srcs = [
        "simulated_vcu_test.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "//external:gflags",
        "//packages/core/test:common",
        "//packages/net",
        "//packages/unity_plugins/simulated_vcu",
        "@gtest//:main",
    ],
)

cc_test(
    name = "network_test",
    srcs = [
        "simulated_vcu_network_test.cpp",
    ],
    tags = [
        "network_test",
        "sim_plugin",
    ],
    deps = [
        "//external:gflags",
        "//packages/core/test:common",
        "//packages/net",
        "//packages/unity_plugins/simulated_vcu",
        "@gtest//:main",
    ],
)
