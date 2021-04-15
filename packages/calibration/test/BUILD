load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "calibration_test",
    srcs = [
        "kannala_brandt_distortion_model_test.cpp",
        "kb4_image_undistortion_test.cpp",
        "linear_camera_model_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//external:eigen",
        "//external:gflags",
        "//packages/calibration",
        "//packages/hal/proto:camera_sample",
        "//thirdparty/Calibu",
        "@gtest//:main",
    ],
)
