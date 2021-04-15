cc_library(
    name = "object_tracking",
    srcs = ["src/state.cpp"],
    hdrs = [
        "include/algorithm.h",
        "include/state.h",
        "include/track.h",
        "include/track_manager.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//packages/core",
    ],
)

cc_library(
    name = "sbbc_tracking",
    hdrs = [
        "include/sbbc_algorithm.h",
        "include/sbbc_track.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":object_tracking",
        "//packages/calibration/proto:coordinate_transformation",
        "//packages/core/proto:timestamp",
        "//packages/hal/proto:camera_sample",
        "//packages/hal/proto:device",
        "//packages/machine_learning:detection_utils",
        "//packages/perception/proto:detection",
    ],
)
