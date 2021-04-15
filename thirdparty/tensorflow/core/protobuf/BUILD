package(default_visibility = ["//visibility:public"])

load("//protobuf_rules/cpp:rules.bzl", "cc_proto_library")
load("//protobuf_rules/python:rules.bzl", "py_proto_library")

cc_proto_library(
    name = "protobuf_protos",
    protos = glob(["*.proto"]),
    deps = [
        "//google/protobuf:any_protos",
        "//thirdparty/tensorflow/core/framework:framework_protos",
        "//thirdparty/tensorflow/core/lib/core:lib_core_protos",
    ],
)
