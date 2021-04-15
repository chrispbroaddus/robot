load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "feature_detectors",
    srcs = [
        "src/feature_point_selector.cpp",
        "src/harris_feature_detector.cpp",
    ],
    hdrs = [
        "include/feature_point_selector.h",
        "include/harris_feature_detector.h",
        "include/harris_feature_detector_details.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//packages/core",
        "//packages/feature_detectors/proto:feature_point",
    ],
)
