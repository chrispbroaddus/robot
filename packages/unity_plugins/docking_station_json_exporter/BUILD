cc_library(
    name = "docking_station_json_exporter",
    srcs = [
        "docking_station_json_export_interface.cpp",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//external:eigen",
        "//external:gflags",
        "//external:glog",
        "//packages/core",
        "//packages/docking/proto:docking_station",
        "//packages/hal",
        "//packages/net",
        "//packages/unity_plugins/utils",
        "//thirdparty/Sophus",
    ],
)
