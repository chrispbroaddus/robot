load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "fiducials",
    srcs = [
        "apriltag_detector.cpp",
    ],
    hdrs = [
        "apriltag_detector.h",
        "fiducial_detector_interface.h",
    ],
    copts = COPTS,
    data = [
        "//:global_configuration",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//external:apriltags",
        "//external:glog",
        "//external:opencv",
        "//packages/calibration/proto:system_calibration",
        "//packages/core",
        "//packages/hal/proto:camera_sample",
        "//packages/perception/fiducials/proto:apriltag_config",
        "//packages/perception/fiducials/proto:apriltag_detector_options",
        "//packages/utilities/opencv:opencv_utilities",
        "@libdc1394//:dc1394",
    ],
)
