package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "mercury",
    protos = [
        "hardware_requirement.proto",
        "managed_process.proto",
        "system_description.proto",
        "watchdog_policy.proto",
    ],
)

proto_library_bundle(
    name = "connections",
    protos = [
        "connections.proto",
    ],
)
