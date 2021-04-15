load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "test",
    srcs = [
        "convex_hull_test.cpp",
        "quaternion_test.cpp",
        "se3_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/math/geometry",
        "//packages/math/geometry:convex_hull",
        "@gtest//:main",
    ],
)
