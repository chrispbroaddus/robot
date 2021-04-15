load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "FilterGraphTest",
    srcs = [
        "container_test.cpp",
        "filter_test.cpp",
    ],
    copts = COPTS,
    tags = ["manual"],
    deps = [
        "//external:gflags",
        "//packages/filter_graph",
        "@gtest//:main",
    ],
)
