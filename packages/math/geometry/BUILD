load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "geometry",
    hdrs = [
        "quaternion.h",
        "se3.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:eigen",
        "//packages/calibration/proto:coordinate_transformation",
        "//thirdparty/Sophus",
    ],
)

cc_library(
    name = "convex_hull",
    hdrs = [
        "convex_hull.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//external:eigen",
    ],
)
