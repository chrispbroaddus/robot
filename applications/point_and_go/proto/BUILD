package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "point_and_go_options",
    protos = ["point_and_go_options.proto"],
)

proto_library_bundle(
    name = "docking_options",
    protos = ["docking_options.proto"],
    deps = ["//packages/teleop/proto:connection_options"],
)
