package(default_visibility = ["//visibility:public"])

load("//protobuf_rules/cpp:rules.bzl", "cc_proto_library")
load("//protobuf_rules/python:rules.bzl", "py_proto_library")

cc_proto_library(
    name = "ccSampleMessage",
    protos = ["SampleMessage.proto"],
)

py_proto_library(
    name = "pySampleMessage",
    protos = ["SampleMessage.proto"],
    srcs_version = "PY2AND3",
    deps = ["@com_github_google_protobuf//:protobuf_python"],
)

cc_binary(
    name = "ccProtoBufSample",
    srcs = ["main.cpp"],
    deps = [
        ":ccSampleMessage",
        "//external:glog",
        "//external:protobuf_clib",
    ],
)

py_binary(
    name = "pyProtoBufSample",
    srcs = ["main.py"],
    default_python_version = "PY3",
    main = "main.py",
    srcs_version = "PY3",
    deps = [":pySampleMessage"],
)
