package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "ground_truth_vehicle_pose",
    protos = ["ground_truth_vehicle_pose.proto"],
    tags = ["sim_plugin"],
    deps = ["//packages/calibration/proto:coordinate_transformation"],
)

proto_library_bundle(
    name = "unity_telemetry_envelope",
    protos = ["unity_telemetry_envelope.proto"],
    tags = ["sim_plugin"],
    deps = [
        "ground_truth_vehicle_pose",
        "//packages/core/proto:timestamp",
    ],
)
