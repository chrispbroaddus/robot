load("//tools:cpp_compile_flags.bzl", "COPTS")

COMMON_COPTS = COPTS

cc_test(
    name = "perception_test",
    srcs = ["perception_test.cpp"],
    copts = COMMON_COPTS,
    deps = [
        "//external:gflags",
        "//packages/core/proto:geometry",
        "//packages/perception",
        "//packages/perception/proto:perception_options",
        "@gtest//:main",
    ],
)

cc_test(
    name = "grid_test",
    srcs = ["grid_test.cpp"],
    copts = COMMON_COPTS,
    deps = [
        "//external:gflags",
        "//packages/perception",
        "@gtest//:main",
    ],
)

cc_test(
    name = "utils_test",
    srcs = ["utils_test.cpp"],
    copts = COMMON_COPTS,
    deps = [
        "//external:gflags",
        "//packages/perception",
        "@gtest//:main",
    ],
)

cc_test(
    name = "utils_performance_test",
    srcs = ["utils_performance_test.cpp"],
    copts = COMMON_COPTS,
    tags = ["manual"],
    deps = [
        "//external:gflags",
        "//packages/perception",
        "@gtest//:main",
    ],
)

cc_test(
    name = "terrain_test",
    srcs = ["terrain_test.cpp"],
    copts = COMMON_COPTS,
    deps = [
        "//external:gflags",
        "//packages/perception",
        "@gtest//:main",
    ],
)

cc_test(
    name = "logging_test",
    srcs = ["logging_test.cpp"],
    copts = COMMON_COPTS,
    deps = [
        "//external:gflags",
        "//packages/hal/proto:camera_sample",
        "//packages/perception",
        "@gtest//:main",
    ],
)
