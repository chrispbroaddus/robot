load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "core",
    srcs = [],
    hdrs = [
        "include/aligned_memory_allocator.h",
        "include/chrono.h",
        "include/dynamic_image.h",
        "include/dynamic_storage.h",
        "include/image_view.h",
        "include/pixel_layout.h",
        "include/pose_interpolator.h",
        "include/producer_consumer_queue.h",
        "include/unmanaged_storage.h",
        "include/wait_queue.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:eigen",
        "//thirdparty/Sophus",
    ],
)
