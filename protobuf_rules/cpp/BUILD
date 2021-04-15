package(default_visibility = ["//visibility:public"])

load("//protobuf_rules/protobuf:rules.bzl", "proto_language")

proto_language(
    name = "cpp",
    pb_compile_deps = [
        "//external:protobuf_clib",
    ],
    pb_file_extensions = [
        ".pb.h",
        ".pb.cc",
    ],
)
