load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "filter_graph",
    srcs = ["src/filter.cpp"],
    hdrs = [
        "include/aggregate_sample.h",
        "include/container.h",
        "include/filter.h",
        "include/sample.h",
        "include/sink_filter.h",
        "include/source_filter.h",
        "include/thread_pool.h",
        "include/transform_filter.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/core",
    ],
)
