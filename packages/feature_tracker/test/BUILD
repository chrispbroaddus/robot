load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "feature_tracker_test",
    srcs = [
        "epipolar_computation_utils_test.cpp",
        "epipolar_feature_matcher_test.cpp",
        "feature_track_index_test.cpp",
        "mutual_match_feature_test.cpp",
        "ncc_feature_matcher_test.cpp",
        "ncc_feature_store_test.cpp",
        "stereo_feature_matcher_test.cpp",
        "stereo_feature_tracker_test.cpp",
        "track_database_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//packages/core",
        "//packages/feature_detectors/proto:feature_point",
        "//packages/feature_tracker",
        "@gtest//:main",
    ],
)
