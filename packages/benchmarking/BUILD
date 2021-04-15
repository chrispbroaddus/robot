load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "benchmarking",
    srcs = glob([
        "src/*.cpp",
        "src/*.h",
    ]),
    hdrs = glob(["include/*.h"]),
    copts = COPTS,
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "benchmarking_test",
    size = "small",
    srcs = glob([
        "test/*.cpp",
        "test/*.h",
    ]),
    copts = COPTS,
    tags = [],
    visibility = ["//visibility:public"],
    deps = [
        ":benchmarking",
        "@gtest//:main",
    ],
)

cc_binary(
    name = "example_benchmark",
    srcs = glob([
        "example/*.h",
        "example/*.cpp",
    ]),
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":benchmarking",
        "//packages/dense_mapping",
        "//packages/logging",
    ],
)
