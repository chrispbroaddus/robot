package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "metrics",
    protos = ["metrics.proto"],
    deps = [
        "//packages/perception/proto:dataset",
        "//packages/perception/proto:detection",
    ],
)
