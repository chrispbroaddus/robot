load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "video_viewer_test",
    srcs = [
        "video_viewer_test.cpp",
    ],
    copts = COPTS,
    tags = ["manual"],
    deps = [
        "//packages/hal",
        "//packages/videoviewer:libvideoviewer",
        "@gtest//:main",
    ],
)
