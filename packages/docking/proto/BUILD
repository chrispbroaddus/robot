load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "docking_station",
    protos = ["docking_station.proto"],
    visibility = ["//visibility:public"],
    deps = [
        "//packages/calibration/proto:coordinate_transformation",
        "//packages/perception/fiducials/proto:fiducial_configuration",
    ],
)

proto_library_bundle(
    name = "station_search_envelope",
    protos = ["station_search_envelope.proto"],
    visibility = ["//visibility:public"],
    deps = [
        "//packages/hal/proto:gps_telemetry",
        "//packages/perception/fiducials/proto:fiducial_poses",
    ],
)

proto_library_bundle(
    name = "vehicle_calibration",
    protos = [
        "vehicle_calibration.proto",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//packages/calibration/proto:coordinate_transformation",
    ],
)

proto_library_bundle(
    name = "inverse_kinematic_docker_options",
    protos = [
        "inverse_kinematic_docker_options.proto",
    ],
    visibility = ["//visibility:public"],
)
