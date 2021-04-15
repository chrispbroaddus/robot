load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_binary(
    name = "dummy_ae_roi_publisher",
    srcs = ["dummy_ae_roi_publisher.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hald:camera_settings_handler",
    ],
)
