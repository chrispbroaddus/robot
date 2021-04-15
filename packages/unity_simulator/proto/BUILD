package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "simulator_settings",
    protos = [
        "application.proto",
        "camera.proto",
        "cameras.proto",
        "ground_truth.proto",
        "networking.proto",
        "scene.proto",
        "simulated_vehicle_model.proto",
        "simulator_settings.proto",
        "vehicle_calibration.proto",
    ],
    deps = [
        "//packages/core/proto:geometry",
        "//packages/core/proto:timestamp",
        "//packages/hal/proto:camera_id",
    ],
)
