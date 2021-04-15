load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "simple_model_inference_test",
    srcs = [
        "simple_model_inference_test.cpp",
    ],
    copts = COPTS,
    data = [
        "//packages/machine_learning/models:download_saved_model_half_plus_two",
    ],
    tags = [
        "manual",
    ],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/core/test:common",
        "//packages/machine_learning:graph_wrapper",
        "@gtest//:main",
        "@libtensorflow//:tensorflow",
    ],
)

cc_test(
    name = "object_detection_test",
    srcs = [
        "object_detection_test.cpp",
    ],
    copts = COPTS,
    data = [
        ":detection_test_image.jpg",
        "//packages/machine_learning/models:download_pretrained_ssd_mobilenet_v1_model_data",
    ],
    tags = [
        "manual",
    ],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//external:opencv",
        "//packages/core/test:common",
        "//packages/hal/proto:camera_sample",
        "//packages/machine_learning:object_detector",
        "//packages/utilities/opencv:opencv_utilities",
        "@gtest//:main",
        "@libtensorflow//:tensorflow",
    ],
)

cc_binary(
    name = "object_detection_loop",
    srcs = [
        "object_detection_loop.cpp",
    ],
    copts = COPTS,
    data = [
        ":detection_test_image.jpg",
        "//packages/machine_learning/models:download_pretrained_ssd_mobilenet_v1_model_data",
    ],
    tags = [
        "manual",
    ],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//external:opencv",
        "//packages/core/test:common",
        "//packages/hal/proto:camera_sample",
        "//packages/machine_learning:object_detector",
        "//packages/utilities/opencv:opencv_utilities",
        "@gtest//:main",
        "@libtensorflow//:tensorflow",
    ],
)
