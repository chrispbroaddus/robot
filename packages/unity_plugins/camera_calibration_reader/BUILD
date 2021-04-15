cc_library(
    name = "camera_calibration_reader",
    srcs = [
        "src/camera_calibration_reader.cpp",
        "src/camera_calibration_reader_interface.cpp",
    ],
    hdrs = [
        "include/camera_calibration_reader.h",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/calibration/proto:system_calibration",
        "//packages/hal/proto:coordinate_frame_id",
        "//packages/hal/utilities:coordinate_frame_id_utils",
        "//packages/unity_plugins/utils",
    ],
)

cc_binary(
    name = "camera_calibration_validator",
    srcs = [
        "src/camera_calibration_validator.cpp",
    ],
    deps = [
        "camera_calibration_reader",
        "//external:glog",
    ],
)
