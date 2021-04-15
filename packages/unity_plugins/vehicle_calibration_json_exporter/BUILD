cc_library(
    name = "vehicle_calibration_json_exporter",
    srcs = [
        "vehicle_calibration_json_exporter.cpp",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//external:eigen",
        "//external:gflags",
        "//external:glog",
        "//packages/calibration/proto:coordinate_transformation",
        "//packages/core",
        "//packages/docking/proto:vehicle_calibration",
        "//packages/hal",
        "//packages/math",
        "//packages/net",
        "//packages/unity_plugins/utils",
    ],
)
