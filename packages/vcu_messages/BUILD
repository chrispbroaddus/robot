load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_inc_library(
    name = "vcu_messages",
    hdrs = glob(["include/**/*.h"]),
    visibility = ["//visibility:public"],
    deps = [],
)

cc_test(
    name = "vcu_messages_test",
    size = "small",
    srcs = glob(["test/**/*.cpp"]),
    copts = COPTS,
    deps = [
        ":vcu_messages",
        "@gtest//:main",
    ],
)
