load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "feature_tracker",
    srcs = [],
    hdrs = [
        "include/epipolar_computation_utils.h",
        "include/epipolar_feature_matcher.h",
        "include/feature_track_index.h",
        "include/mutual_feature_match.h",
        "include/ncc_feature_matcher.h",
        "include/ncc_feature_store.h",
        "include/stereo_feature_matcher.h",
        "include/stereo_feature_track_index.h",
        "include/stereo_feature_tracker.h",
        "include/track_database.h",
        "include/track_database_details.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:eigen",
        "//packages/calibration",
        "//packages/calibration/proto:system_calibration",
        "//packages/core",
        "//packages/feature_detectors",
        "//packages/feature_detectors/proto:feature_point",
    ],
)
