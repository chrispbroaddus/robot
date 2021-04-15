package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "path",
    protos = ["path.proto"],
    deps = ["//packages/calibration/proto:coordinate_transformation"],
)

proto_library_bundle(
    name = "trajectory_planner_options",
    protos = ["trajectory_planner_options.proto"],
    deps = [":trajectory"],
)

proto_library_bundle(
    name = "trajectory_options",
    protos = ["trajectory_options.proto"],
)

proto_library_bundle(
    name = "trajectory",
    protos = ["trajectory.proto"],
    deps = [
        "//packages/core/proto:timestamp",
    ],
)

proto_library_bundle(
    name = "navigator_options",
    protos = ["navigator_options.proto"],
)

proto_library_bundle(
    name = "state",
    protos = ["state.proto"],
)
