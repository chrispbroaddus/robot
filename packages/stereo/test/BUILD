load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "stereo_test",
    srcs = [
        "sync_mux_test.cpp",
    ],
    copts = COPTS,
    tags = ["hardware"],
    deps = [
        "//external:gflags",
        "//packages/stereo",
        "@gtest//:main",
    ],
)
