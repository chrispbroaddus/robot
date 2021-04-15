load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "opencv_utilities",
    srcs = [
        "datatype_conversions.cpp",
    ],
    hdrs = [
        "datatype_conversions.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//external:opencv",
        "//packages/calibration/proto:system_calibration",
        "//packages/hal/proto:camera_sample",
        "@libdc1394//:dc1394",
    ],
)

cc_test(
    name = "opencv_utilities_test",
    srcs = [
        "datatype_conversions_test.cpp",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//packages/utilities/opencv:opencv_utilities",
        "@gtest//:main",
    ],
)
