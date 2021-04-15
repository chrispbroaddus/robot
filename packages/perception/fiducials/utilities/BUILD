load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_binary(
    name = "fiducial_detector",
    srcs = ["fiducial_detector.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hal",
        "//packages/perception/fiducials",
    ],
)
