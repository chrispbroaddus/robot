load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "calibration",
    srcs = [],
    hdrs = [
        "include/kannala_brandt_distortion_model.h",
        "include/kb4_image_undistortion.h",
        "include/linear_camera_model.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:eigen",
    ],
)
