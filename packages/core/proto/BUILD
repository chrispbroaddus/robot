package(default_visibility = ["//visibility:public"])

load("//protobuf_rules:bundle.bzl", "proto_library_bundle")

proto_library_bundle(
    name = "timestamp",
    protos = ["timestamp.proto"],
)

proto_library_bundle(
    name = "geometry",
    protos = ["geometry.proto"],
)

load("@fi_kapsi_jpa_nanopb//tools/nanopb:rules.bzl", "cc_nanopb_library")

cc_nanopb_library(
    name = "nanopb_timestamp",
    srcs = ["timestamp.proto"],
    visibility = ["//visibility:public"],
)
