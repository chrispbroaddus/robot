package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "volumetric_time_series_dataset",
    protos = ["volumetric_time_series_dataset.proto"],
    deps = [
        "//packages/calibration/proto:coordinate_transformation",
        "//packages/core/proto:geometry",
        "//packages/core/proto:timestamp",
    ],
)
