package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "executor_options",
    protos = ["executor_options.proto"],
)

proto_library_bundle(
    name = "executor_response",
    protos = ["executor_response.proto"],
)

proto_library_bundle(
    name = "state",
    protos = ["state.proto"],
)

proto_library_bundle(
    name = "log",
    protos = ["log.proto"],
    deps = [
        ":state",
        "//packages/core/proto:geometry",
        "//packages/core/proto:timestamp",
        "//packages/planning/proto:state",
        "//packages/planning/proto:trajectory",
    ],
)
