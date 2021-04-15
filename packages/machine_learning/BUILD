load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "graph_wrapper",
    srcs = [
        "src/graph_wrapper.cpp",
    ],
    hdrs = [
        "include/graph_wrapper.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//thirdparty/tensorflow/core/protobuf:protobuf_protos",
        "@libtensorflow//:tensorflow",
    ],
)

cc_library(
    name = "object_detector",
    srcs = [
        "src/object_detector.cpp",
    ],
    hdrs = [
        "include/object_detector.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":graph_wrapper",
        "//external:gflags",
        "//external:glog",
        "//packages/hal/proto:camera_sample",
        "//packages/perception/proto:detection",
    ],
)

cc_library(
    name = "object_detection_runner",
    srcs = [
        "src/object_detection_runner.cpp",
    ],
    hdrs = [
        "include/object_detection_runner.h",
    ],
    copts = COPTS,
    data = [
        "//:global_configuration",
        "//packages/machine_learning/models:download_pretrained_ssd_mobilenet_v1_model_data",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//external:cppzmq",
        "//external:gflags",
        "//external:glog",
        "//packages/calibration",
        "//packages/calibration/proto:system_calibration",
        "//packages/hal/proto:camera_id",
        "//packages/hal/proto:camera_sample",
        "//packages/machine_learning:object_detector",
        "//packages/machine_learning/proto:object_detection_runner_options",
        "//packages/net",
        "//packages/object_tracking:sbbc_tracking",
    ],
)

cc_library(
    name = "detection_utils",
    srcs = [
        "src/detection_utils.cpp",
    ],
    hdrs = [
        "include/detection_utils.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/perception/proto:detection",
    ],
)

cc_library(
    name = "object_volume_estimator",
    srcs = [
        "src/object_volume_estimator_runner.cpp",
    ],
    hdrs = [
        "include/object_volume_estimator.h",
        "include/object_volume_estimator_runner.h",
        "include/rgb_pointcloud_fusion_3d_bbox_estimator.h",
        "include/rgb_pointcloud_fusion_3d_convex_hull_estimator.h",
    ],
    data = [
        "//:global_configuration",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//external:cppzmq",
        "//external:eigen",
        "//external:gflags",
        "//external:glog",
        "//packages/calibration",
        "//packages/calibration/proto:system_calibration",
        "//packages/hal/proto:camera_id",
        "//packages/hal/proto:camera_sample",
        "//packages/machine_learning/proto:object_volume_estimator_runner_options",
        "//packages/math/geometry:convex_hull",
        "//packages/net",
        "//packages/perception/proto:detection",
    ],
)
