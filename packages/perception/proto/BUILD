package(default_visibility = ["//visibility:public"])

#load("//protobuf_rules/cpp:rules.bzl", "cc_proto_library")
load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "perception_options",
    protos = ["perception_options.proto"],
)

proto_library_bundle(
    name = "grid_options",
    protos = ["grid_options.proto"],
)

proto_library_bundle(
    name = "grid",
    protos = ["grid.proto"],
    deps = [":grid_options"],
)

proto_library_bundle(
    name = "terrain_options",
    protos = ["terrain_options.proto"],
    deps = [":grid_options"],
)

proto_library_bundle(
    name = "detection",
    protos = ["detection.proto"],
    deps = [
        "//packages/calibration/proto:coordinate_transformation",
        "//packages/core/proto:timestamp",
        "//packages/hal/proto:device",
    ],
)

proto_library_bundle(
    name = "dataset",
    protos = ["dataset.proto"],
    deps = [
        ":detection",
    ],
)
