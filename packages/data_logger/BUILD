load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_binary(
    name = "data_logger",
    srcs = [
        "include/data_logger.h",
        "include/data_logger_graph.h",
        "include/data_logger_sample.h",
        "include/device_file_sink_filter.h",
        "include/device_source_filter.h",
        "include/sdl_render_sink_filter.h",
        "src/data_logger.cpp",
        "src/data_logger_graph.cpp",
        "src/data_logger_main.cpp",
        "src/device_file_sink_filter.cpp",
        "src/device_source_filter.cpp",
        "src/sdl_render_sink_filter.cpp",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:SDL",
        "//external:gflags",
        "//external:glog",
        "//packages/core",
        "//packages/data_logger/proto:config",
        "//packages/filter_graph",
        "//packages/hal",
        "//packages/hald:haldlib",
        "//packages/image_codec",
        "//packages/serialization",
        "//packages/videoviewer:libvideoviewer",
    ],
)
