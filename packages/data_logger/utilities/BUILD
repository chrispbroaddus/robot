load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_binary(
    name = "get_protodat_stats",
    srcs = [
        "get_protodat_stats.cpp",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/data_logger/proto:config",
        "//packages/filesystem",
        "//packages/hal",
        "//packages/image_codec",
        "//packages/serialization",
    ],
)

cc_binary(
    name = "protodat_to_image_files",
    srcs = [
        "protodat_to_image_files.cpp",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/data_logger/proto:config",
        "//packages/filesystem",
        "//packages/hal",
        "//packages/serialization",
    ],
)
