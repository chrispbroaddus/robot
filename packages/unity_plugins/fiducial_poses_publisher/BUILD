cc_library(
    name = "fiducial_poses_publisher",
    srcs = [
        "src/fiducial_poses_publisher.cpp",
    ],
    hdrs = [
        "include/fiducial_poses_publisher.h",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//external:eigen",
        "//external:gflags",
        "//external:glog",
        "//packages/core",
        "//packages/hal",
        "//packages/net",
        "//packages/perception/fiducials/proto:fiducial_poses",
        "//packages/unity_plugins/utils",
    ],
)
