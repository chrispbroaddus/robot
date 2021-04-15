package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "apriltag_config",
    protos = ["apriltag_config.proto"],
)

proto_library_bundle(
    name = "apriltag_detector_options",
    protos = ["apriltag_detector_options.proto"],
)

proto_library_bundle(
    name = "fiducial_configuration",
    protos = ["fiducial_configuration.proto"],
    deps = ["//packages/perception/fiducials/proto:apriltag_config"],
)

proto_library_bundle(
    name = "fiducial_poses",
    protos = ["fiducial_poses.proto"],
    deps = [
        "fiducial_configuration",
        "//packages/calibration/proto:coordinate_transformation",
    ],
)
