load("//tools:cpp_compile_flags.bzl", "COPTS")

COMMON_COPTS = COPTS

cc_test(
    name = "estimator_test",
    srcs = ["estimator_test.cpp"],
    copts = COMMON_COPTS,
    deps = [
        "//packages/core/test:common",
        "//packages/estimation",
        "@gtest//:main",
    ],
)
