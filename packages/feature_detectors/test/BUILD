load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "feature_detectors_test",
    srcs = [
        "feature_point_selection_test.cpp",
        "harris_feature_detector_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//packages/feature_detectors",
        "@gtest//:main",
    ],
)
