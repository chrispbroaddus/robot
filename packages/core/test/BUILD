load("//protobuf_rules/python:rules.bzl", "py_proto_library")
load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "core_test",
    srcs = [
        "chrono_test.cpp",
        "dynamic_image_test.cpp",
        "pose_interpolator_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//external:gflags",
        "//packages/core",
        "@gtest//:main",
    ],
)

cc_library(
    name = "common",
    hdrs = [
        "common.h",
    ],
    copts = COPTS,
    linkopts = [
        "-lrt",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "@boost//:interprocess",
    ],
)

cc_test(
    name = "common_test",
    srcs = [
        "common_test.cpp",
    ],
    copts = COPTS,
    deps = [
        ":common",
        "//external:zmq",
        "@boost//:interprocess",
        "@gtest//:main",
    ],
)

py_proto_library(
    name = "dummy",
    protos = ["dummy.proto"],
    srcs_version = "PY2AND3",
)

py_test(
    name = "python_proto_test",
    srcs = ["python_proto_test.py"],
    default_python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        ":dummy",
        "@com_github_google_protobuf//:protobuf_python",
    ],
)
