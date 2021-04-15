load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "logging_test",
    srcs = [
        "logging_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//packages/executor:logging",
        "//packages/planning:logging",
        "@gtest//:main",
    ],
)

cc_test(
    name = "executor_test",
    srcs = [
        "executor_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//packages/executor:execution",
        "//packages/planning:planner",
        "//packages/planning/test:common",
        "@gtest//:main",
    ],
)
