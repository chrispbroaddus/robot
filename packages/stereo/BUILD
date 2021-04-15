load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "stereo",
    srcs = [
        "src/calibration_file_sink_filter.cpp",
        "src/stereo_calibration_pipeline.cpp",
        "src/stereo_filter.cpp",
        "src/stereo_pipeline.cpp",
        "src/sync_mux_filter.cpp",
        "src/zmq_camera_source_filter.cpp",
        "src/zmq_depth_sink_filter.cpp",
    ],
    hdrs = [
        "include/calibration_file_sink_filter.h",
        "include/camera_sample.h",
        "include/stereo_calibration_pipeline.h",
        "include/stereo_filter.h",
        "include/stereo_pipeline.h",
        "include/sync_mux_filter.h",
        "include/zmq_camera_source_filter.h",
        "include/zmq_depth_sink_filter.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//external:opencv",
        "//packages/calibration/proto:system_calibration",
        "//packages/filter_graph",
        "//packages/hal/proto:camera_sample",
        "//packages/net",
        "//packages/utilities/opencv:opencv_utilities",
        "@libdc1394//:dc1394",
    ],
)

cc_binary(
    name = "stereo_app",
    srcs = ["src/stereo_app_main.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//packages/stereo",
    ],
)

cc_binary(
    name = "capture_stereo_calibration_images",
    srcs = ["src/capture_stereo_calibration_images.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//packages/stereo",
    ],
)
