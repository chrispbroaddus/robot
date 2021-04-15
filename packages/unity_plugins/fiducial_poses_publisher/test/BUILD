cc_test(
    name = "test",
    srcs = [
        "fiducial_poses_publisher_test.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "//external:gflags",
        "//packages/core/test:common",
        "//packages/net",
        "//packages/unity_plugins/fiducial_poses_publisher",
        "//packages/unity_plugins/utils",
        "@gtest//:main",
    ],
)

cc_test(
    name = "network_test",
    srcs = [
        "fiducial_poses_publisher_network_test.cpp",
    ],
    tags = [
        "network_test",
    ],
    deps = [
        "//external:gflags",
        "//packages/core/test:common",
        "//packages/net",
        "//packages/unity_plugins/fiducial_poses_publisher",
        "//packages/unity_plugins/utils",
        "@gtest//:main",
    ],
)

cc_binary(
    name = "test_subscriber",
    srcs = [
        "test_subscriber.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/net",
        "//packages/perception/fiducials/proto:fiducial_poses",
        "@gtest//:main",
    ],
)
