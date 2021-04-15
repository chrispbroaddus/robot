load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_binary(
    name = "capture_calibration_images",
    srcs = ["capture_calibration_images.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//external:opencv",
        "//packages/hal",
        "//packages/utilities/opencv:opencv_utilities",
    ],
)

cc_binary(
    name = "undistort_images",
    srcs = ["undistort_images.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:gflags",
        "//external:opencv",
        "//packages/calibration",
        "//packages/hal",
        "//packages/utilities/opencv:opencv_utilities",
    ],
)
