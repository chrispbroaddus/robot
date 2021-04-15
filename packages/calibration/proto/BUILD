package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "coordinate_transformation",
    protos = [
        "coordinate_frame.proto",
        "coordinate_transformation.proto",
    ],
    deps = [
        "//packages/core/proto:timestamp",
        "//packages/hal/proto:device",
    ],
)

proto_library_bundle(
    name = "system_calibration",
    protos = [
        "camera_intrinsic_calibration.proto",
        "kannala_brandt_distortion_model.proto",
        "pinhole_camera_distortion_model.proto",
        "system_calibration.proto",
    ],
    deps = [
        "coordinate_transformation",
        "//packages/core/proto:timestamp",
        "//packages/hal/proto:device",
    ],
)
