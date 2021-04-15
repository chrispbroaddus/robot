load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "libvideoviewer",
    srcs = [
        "src/sdl_window.cpp",
        "src/video_viewer.cpp",
    ],
    hdrs = [
        "include/sdl_window.h",
        "include/video_viewer.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:SDL",
        "//external:gflags",
        "//external:glog",
        "//packages/core",
        "//packages/hal/proto:camera_sample",
        "//packages/net",
    ],
)

cc_binary(
    name = "videoviewer",
    srcs = [
        "src/main.cpp",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//packages/videoviewer:libvideoviewer",
    ],
)
