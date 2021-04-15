cc_library(
    name = "synthetic_joystick_publisher",
    srcs = [
        "synthetic_joystick_publisher.cpp",
    ],
    hdrs = [
        "synthetic_joystick_publisher.h",
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

cc_binary(
    name = "runSyntheticJoystickPublisher",
    srcs = [
        "run_synthetic_joystick_publisher.cpp",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "synthetic_joystick_publisher",
        "//external:gflags",
        "//external:glog",
        "//packages/core",
        "//packages/hal",
        "//packages/net",
        "//packages/unity_plugins/utils",
    ],
)

cc_test(
    name = "test",
    srcs = [
        "joystick_subscriber_test.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "synthetic_joystick_publisher",
        "//external:gflags",
        "//packages/net",
        "//packages/unity_plugins/joystick_subscriber",
        "@gtest//:main",
    ],
)

cc_test(
    name = "network_test",
    srcs = [
        "joystick_subscriber_network_test.cpp",
    ],
    tags = [
        "network_test",
    ],
    deps = [
        "synthetic_joystick_publisher",
        "//external:gflags",
        "//packages/net",
        "//packages/unity_plugins/joystick_subscriber",
        "@gtest//:main",
    ],
)
