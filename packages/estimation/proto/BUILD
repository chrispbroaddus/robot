package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "estimator_options",
    protos = ["estimator_options.proto"],
)

proto_library_bundle(
    name = "state",
    protos = ["state.proto"],
    deps = [
        "//packages/calibration/proto:coordinate_transformation",
        "//packages/core/proto:timestamp",
    ],
)
