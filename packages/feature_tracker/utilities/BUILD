load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_binary(
    name = "epipolar_line_visualizer",
    srcs = ["epipolar_line_visualizer.cpp"],
    copts = COPTS,
    deps = [
        "//packages/feature_detectors",
        "//packages/feature_tracker",
        "//packages/net",
        "//packages/utilities/opencv:opencv_utilities",
        "//packages/videoviewer:libvideoviewer",
    ],
)
