cc_library(
    name = "simulated_vcu",
    srcs = [
        "src/simulated_vcu.cpp",
    ],
    hdrs = [
        "include/simulated_vcu.h",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/core",
        "//packages/hal",
        "//packages/net",
        "//packages/planning:definitions",
        "//packages/planning:vcu_interop",
        "//packages/unity_plugins/simulator_settings_reader",
        "//packages/unity_plugins/utils",
    ],
)

cc_binary(
    name = "test_trajectory_publisher",
    srcs = [
        "src/test_trajectory_publisher.cpp",
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
