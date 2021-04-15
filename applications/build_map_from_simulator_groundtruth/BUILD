load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_binary(
    name = "build_map_from_simulator_groundtruth",
    srcs = glob([
        "src/*.cpp",
        "src/*.h",
    ]),
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/benchmarking",
        "//packages/core",
        "//packages/dense_mapping",
        "//packages/dense_mapping/proto:volumetric_time_series_dataset",
        "//packages/estimation/proto:state",
        "//packages/hal",
        "//packages/hal/proto:camera_sample",
        "//packages/math",
        "//packages/serialization",
        "//packages/unity_plugins/proto:unity_telemetry_envelope",
        "//thirdparty/Sophus",
        "//thirdparty/gflags",
    ],
)
