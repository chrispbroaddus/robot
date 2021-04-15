load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "hald_test",
    srcs = [],
    copts = COPTS,
    tags = ["hardware"],
    deps = [
        "//external:gflags",
        "//packages/hald:haldlib",
        "@gtest//:main",
    ],
)
