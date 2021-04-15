load("//tools:cpp_compile_flags.bzl", "COPTS")

COMMON_COPTS = COPTS

cc_library(
    name = "common",
    srcs = [
        "common.h",
    ],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//packages/estimation",
    ],
)

cc_test(
    name = "navigator_test",
    srcs = [
        "common.h",
        "navigator_test.cpp",
    ],
    copts = COMMON_COPTS,
    deps = [
        "//packages/estimation",
        "//packages/perception",
        "//packages/planning:planner",
        "@gtest//:main",
    ],
)

cc_test(
    name = "planner_test",
    srcs = [
        "common.h",
        "planner_test.cpp",
    ],
    copts = COMMON_COPTS,
    deps = [
        "//packages/planning:planner",
        "@gtest//:main",
    ],
)

cc_test(
    name = "utils_test",
    srcs = [
        "common.h",
        "utils_test.cpp",
    ],
    copts = COMMON_COPTS,
    deps = [
        "//packages/planning:utils",
        "@gtest//:main",
    ],
)

cc_test(
    name = "vcu_interop_test",
    srcs = [
        "common.h",
        "vcu_interop_test.cpp",
    ],
    copts = COMMON_COPTS,
    deps = [
        "//packages/planning:planner",
        "//packages/planning:utils",
        "//packages/planning:vcu_interop",
        "@gtest//:main",
    ],
)

cc_test(
    name = "trajectory_test",
    srcs = [
        "common.h",
        "trajectory_test.cpp",
    ],
    copts = COMMON_COPTS,
    deps = [
        "//packages/planning:planner",
        "@gtest//:main",
    ],
)
