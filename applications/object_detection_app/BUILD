load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_binary(
    name = "object_detection_app",
    srcs = [
        "object_detector_app.cpp",
    ],
    copts = COPTS,
    data = [
        "//packages/machine_learning/models:download_pretrained_ssd_mobilenet_v1_model_data",
    ],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/calibration",
        "//packages/calibration/proto:system_calibration",
        "//packages/hal/proto:camera_id",
        "//packages/machine_learning:object_detection_runner",
    ],
)

cc_binary(
    name = "object_detection_viewer",
    srcs = [
        "object_detection_viewer.cpp",
    ],
    copts = COPTS,
    deps = [
        "//external:cppzmq",
        "//external:gflags",
        "//external:glog",
        "//packages/calibration",
        "//packages/calibration/proto:system_calibration",
        "//packages/core",
        "//packages/hal/proto:camera_id",
        "//packages/hal/proto:camera_sample",
        "//packages/machine_learning:object_detector",
        "//packages/net",
        "//packages/videoviewer:libvideoviewer",
    ],
)

cc_binary(
    name = "object_volume_estimator_app",
    srcs = [
        "object_volume_estimator_app.cpp",
    ],
    deps = [
        "//external:cppzmq",
        "//external:gflags",
        "//external:glog",
        "//packages/calibration",
        "//packages/calibration/proto:system_calibration",
        "//packages/machine_learning:object_detector",
        "//packages/machine_learning:object_volume_estimator",
        "//packages/machine_learning/proto:object_volume_estimator_runner_options",
    ],
)
