load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "triangulation",
    hdrs = [
        "include/triangulation.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:eigen",
    ],
)

cc_test(
    name = "triangulation_test",
    srcs = [
        "test/triangulation_test.cpp",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//packages/triangulation",
        "@gtest//:main",
    ],
)
