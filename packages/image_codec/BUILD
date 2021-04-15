load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "image_codec",
    srcs = [
        "src/jpeg.cpp",
        "src/jpeg/jpeg_decoder.cpp",
        "src/jpeg/jpeg_encoder.cpp",
        "src/passthrough/passthrough_decoder.cpp",
        "src/passthrough/passthrough_encoder.cpp",
        "src/png.cpp",
        "src/png/png_decoder.cpp",
        "src/png/png_encoder.cpp",
    ],
    hdrs = [
        "include/image_decoder_interface.h",
        "include/image_encoder_interface.h",
        "include/jpeg.h",
        "include/jpeg/jpeg_decoder.h",
        "include/jpeg/jpeg_encoder.h",
        "include/passthrough/passthrough_decoder.h",
        "include/passthrough/passthrough_encoder.h",
        "include/png.h",
        "include/png/png_decoder.h",
        "include/png/png_encoder.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/core",
        "//packages/hal/proto:camera_sample",
        "@jpeg_archive//:jpeg",
        "@png_archive//:png",
    ],
)

cc_test(
    name = "jpeg_test",
    srcs = ["test/jpeg_test.cpp"],
    copts = COPTS,
    data = [":image_codec_test_files"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//external:opencv",
        "//packages/image_codec",
        "@gtest//:main",
    ],
)

cc_test(
    name = "png_test",
    srcs = ["test/png_test.cpp"],
    copts = COPTS,
    data = [":image_codec_test_files"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//external:opencv",
        "//packages/image_codec",
        "@gtest//:main",
    ],
)

cc_test(
    name = "passthrough_codec_test",
    srcs = ["test/passthrough_test.cpp"],
    copts = COPTS,
    data = [":image_codec_test_files"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//external:opencv",
        "//packages/image_codec",
        "@gtest//:main",
    ],
)

filegroup(
    name = "image_codec_test_files",
    srcs = [
        "test/test.png",
    ],
)
