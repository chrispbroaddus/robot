package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "coordinate_frame_id",
    protos = ["coordinate_frame_id.proto"],
)

proto_library_bundle(
    name = "camera_id",
    protos = ["camera_id.proto"],
)

proto_library_bundle(
    name = "device",
    protos = ["device.proto"],
)

proto_library_bundle(
    name = "camera_sample",
    protos = ["camera_sample.proto"],
    deps = [
        "//packages/core/proto:timestamp",
        "//packages/hal/proto:device",
    ],
)

proto_library_bundle(
    name = "camera_trigger_sample",
    protos = ["camera_trigger_sample.proto"],
    deps = [
        "//packages/core/proto:timestamp",
        "//packages/hal/proto:camera_id",
    ],
)

proto_library_bundle(
    name = "imu_sample",
    protos = ["imu_sample.proto"],
    deps = [
        "//packages/core/proto:timestamp",
        "//packages/hal/proto:device",
    ],
)

proto_library_bundle(
    name = "joystick_sample",
    protos = ["joystick_sample.proto"],
    deps = [
        "//packages/core/proto:timestamp",
        "//packages/hal/proto:device",
    ],
)

proto_library_bundle(
    name = "vehicle_telemetry_sample",
    protos = ["vehicle_telemetry_sample.proto"],
    deps = [
        "//packages/core/proto:timestamp",
        "//packages/hal/proto:device",
    ],
)

proto_library_bundle(
    name = "point_and_go_sample",
    protos = ["point_and_go_sample.proto"],
    deps = [
        "//packages/core/proto:timestamp",
        "//packages/hal/proto:device",
    ],
)

proto_library_bundle(
    name = "ae_roi_sample",
    protos = ["auto_exposure_roi_sample.proto"],
    deps = [
        "//packages/core/proto:timestamp",
        "//packages/hal/proto:device",
    ],
)

proto_library_bundle(
    name = "simulator_commands",
    protos = [
        "simulator_camera_command.proto",
        "simulator_camera_output.proto",
        "simulator_camera_response.proto",
        "simulator_command_envelope.proto",
        "simulator_command_response.proto",
        "simulator_reset_command.proto",
        "simulator_reset_response.proto",
        "simulator_stats_command.proto",
        "simulator_stats_response.proto",
    ],
    deps = [
        "//packages/hal/proto:camera_id",
    ],
)

proto_library_bundle(
    name = "simulator_stats",
    protos = [
        "simulator_stats_telemetry.proto",
    ],
)

proto_library_bundle(
    name = "vcu_commands",
    protos = [
        "vcu_command_envelope.proto",
        "vcu_command_response.proto",
        "vcu_emergency_stop_command.proto",
        "vcu_idle_command.proto",
        "vcu_ik_control_command.proto",
        "vcu_trajectory_command.proto",
    ],
    deps = [
        "//packages/core/proto:timestamp",
    ],
)

proto_library_bundle(
    name = "vcu_requests",
    protos = [
        "vcu_status_request.proto",
        "vcu_status_response.proto",
    ],
    deps = [
        "//packages/core/proto:timestamp",
    ],
)

proto_library_bundle(
    name = "vcu_telemetry",
    protos = [
        "vcu_imu_telemetry.proto",
        "vcu_servo_telemetry.proto",
        "vcu_slider_telemetry.proto",
        "vcu_suspension_encoder_telemetry.proto",
        "vcu_telemetry_envelope.proto",
        "vcu_wheel_encoder_telemetry.proto",
    ],
    deps = ["//packages/core/proto:timestamp"],
)

proto_library_bundle(
    name = "gps_telemetry",
    protos = ["gps_telemetry.proto"],
    deps = [
        "//packages/core/proto:timestamp",
    ],
)

proto_library_bundle(
    name = "network_telemetry",
    protos = ["network_health_telemetry.proto"],
    deps = [
        "//packages/core/proto:timestamp",
    ],
)

# Nanopb
load("@fi_kapsi_jpa_nanopb//tools/nanopb:rules.bzl", "cc_nanopb_library")

cc_nanopb_library(
    name = "nanopb_vcu",
    srcs = [
        "camera_id.proto",
        "camera_trigger_sample.proto",
        "vcu_command_envelope.proto",
        "vcu_command_response.proto",
        "vcu_emergency_stop_command.proto",
        "vcu_idle_command.proto",
        "vcu_ik_control_command.proto",
        "vcu_imu_telemetry.proto",
        "vcu_servo_telemetry.proto",
        "vcu_slider_telemetry.proto",
        "vcu_status_request.proto",
        "vcu_status_response.proto",
        "vcu_suspension_encoder_telemetry.proto",
        "vcu_telemetry_envelope.proto",
        "vcu_trajectory_command.proto",
        "vcu_wheel_encoder_telemetry.proto",
    ],
    options = [
        "camera_trigger_sample.options",
        "vcu_imu_telemetry.options",
        "vcu_trajectory_command.options",
    ],
    visibility = ["//visibility:public"],
    deps = ["//packages/core/proto:nanopb_timestamp"],
)
