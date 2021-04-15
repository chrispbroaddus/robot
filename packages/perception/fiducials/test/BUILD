load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "apriltag_detector_test",
    srcs = [
        "apriltag_detector_test.cpp",
    ],
    copts = COPTS,
    data = [":apriltag_test_files"],
    deps = [
        "//external:gflags",
        "//packages/perception/fiducials",
        "@gtest//:main",
    ],
)

filegroup(
    name = "apriltag_test_files",
    srcs = [
        "apriltag_test_image.png",
    ],
)

cc_binary(
    name = "apriltag_config_serialize_test",
    srcs = [
        "apriltag_config_serialize_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//packages/perception/fiducials/proto:apriltag_config",
    ],
)

cc_binary(
    name = "apriltag_detector_options_serialize_test",
    srcs = [
        "apriltag_detector_options_serialize_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//packages/perception/fiducials/proto:apriltag_detector_options",
    ],
)
