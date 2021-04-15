load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "serialization",
    srcs = [
        "src/proto.cpp",
    ],
    hdrs = [
        "include/proto.h",
        "include/protobuf_io.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "@com_github_google_protobuf//:protobuf",
    ],
)

cc_test(
    name = "protobuf_io_test",
    srcs = [
        "test/protobuf_io_test.cpp",
    ],
    copts = COPTS,
    tags = ["hardware"],
    deps = [
        "serialization",
        "//packages/core/proto:timestamp",
        "@gtest//:main",
    ],
)
