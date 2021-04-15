package(default_visibility = ["//visibility:public"])

load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "dense_mapping",
    srcs = glob([
        "src/*.cpp",
        "src/*.h",
    ]),
    hdrs = glob(["include/*.h"]),
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = ["//packages/dense_mapping/proto:volumetric_time_series_dataset"],
)

cc_test(
    name = "dense_mapping_test",
    size = "small",
    timeout = "long",
    srcs = glob([
        "test/*.cpp",
        "test/*.h",
    ]),
    copts = COPTS,
    tags = [],
    deps = [
        "dense_mapping",
        "//packages/benchmarking",
        "@gtest//:main",
    ],
)

cc_binary(
    name = "dense_mapping_benchmark",
    srcs = glob([
        "benchmark/*.cpp",
        "benchmark/*.h",
    ]),
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":dense_mapping",
        "//external:glog",
        "//packages/benchmarking",
    ],
)

cc_binary(
    name = "dense_mapping_sphere_visualization",
    srcs = glob([
        "sphere_visualization/*.h",
        "sphere_visualization/*.cpp",
    ]),
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":dense_mapping",
        "//external:glog",
        "//packages/benchmarking",
    ],
)
