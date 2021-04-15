load("//tools:cpp_compile_flags.bzl", "COPTS")

COMMON_COPTS = COPTS

cc_library(
    name = "perception",
    srcs = [
        "grid.cpp",
        "perception.cpp",
        "terrain.cpp",
        "utils.cpp",
    ],
    hdrs = [
        "grid.h",
        "logging.h",
        "perception.h",
        "terrain.h",
        "types.h",
        "utils.h",
    ],
    copts = COMMON_COPTS + ["-fopenmp"],
    linkopts = ["-lgomp"],
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//external:wykobi",
        "//packages/calibration/proto:system_calibration",
        "//packages/core",
        "//packages/core/proto:geometry",
        "//packages/hal/proto:camera_sample",
        "//packages/perception/proto:grid",
        "//packages/perception/proto:grid_options",
        "//packages/perception/proto:perception_options",
        "//packages/perception/proto:terrain_options",
        "//packages/planning:definitions",
        "//packages/planning:utils",
        "//packages/teleop/proto:backend_message",
        "//thirdparty/Calibu",
        "//thirdparty/Sophus",
    ],
)

cc_library(
    name = "trackball",
    srcs = [
        "thirdparty/Camera.cpp",
        "thirdparty/Interactor.cpp",
    ],
    hdrs = [
        "thirdparty/Camera.h",
        "thirdparty/Interactor.h",
    ],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "visualization",
    srcs = ["visualization.cpp"],
    hdrs = ["visualization.h"],
    copts = COMMON_COPTS,
    linkopts = [
        "-lGL",
        "-lGLU",
        "-lGLEW",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":perception",
        "//external:SDL",
        "//thirdparty/Sophus",
    ],
)

cc_library(
    name = "logging",
    hdrs = [
        "logging.h",
    ],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
    ],
)

filegroup(
    name = "bin_shaders",
    srcs = glob(["bin/shaders/*.fragmentshader"]) + glob([
        "bin/shaders/*.vertexshader",
    ]),
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "base_visualizer",
    srcs = ["bin/base_visualizer.cpp"],
    copts = ["-DHAVE_OPENGL"] + COMMON_COPTS,
    data = [":bin_shaders"],
    deps = [
        ":perception",
        ":trackball",
        ":visualization",
        "//external:SDL",
        "//external:glog",
        "//packages/planning:utils",
    ],
)

cc_binary(
    name = "voxelizer",
    srcs = ["bin/voxelizer.cpp"],
    deps = [
        ":perception",
        "//external:glog",
        "//packages/net",
        "//packages/planning:utils",
    ],
)

cc_binary(
    name = "voxel_visualizer",
    srcs = ["bin/voxel_visualizer.cpp"],
    copts = ["-DHAVE_OPENGL"] + COMMON_COPTS,
    data = [":bin_shaders"],
    deps = [
        ":perception",
        ":trackball",
        ":visualization",
        "//external:SDL",
        "//external:glog",
        "//packages/estimation",
        "//packages/estimation:visualization",
        "//packages/net",
        "//packages/planning:utils",
    ],
)

cc_binary(
    name = "stereo_log_writer",
    srcs = ["bin/stereo_log_writer.cpp"],
    copts = COMMON_COPTS,
    deps = [
        ":logging",
        "//external:glog",
        "//packages/hal/proto:camera_sample",
        "//packages/net",
    ],
)

cc_binary(
    name = "stereo_log_reader",
    srcs = ["bin/stereo_log_reader.cpp"],
    copts = COMMON_COPTS,
    deps = [
        ":logging",
        ":perception",
        "//external:glog",
        "//packages/hal/proto:camera_sample",
    ],
)

cc_binary(
    name = "write_dummy_system_calibration",
    srcs = ["bin/write_dummy_system_calibration.cpp"],
    deps = [
        ":perception",
        "//external:glog",
        "//packages/calibration/proto:system_calibration",
        "//packages/hal/proto:device",
    ],
)

cc_binary(
    name = "calibration_visualizer",
    srcs = ["bin/calibration_visualizer.cpp"],
    copts = ["-DHAVE_OPENGL"],
    data = ["//packages/perception:bin_shaders"],
    deps = [
        "//external:SDL",
        "//external:glog",
        "//packages/perception",
        "//packages/perception:trackball",
        "//packages/perception:visualization",
        "//packages/planning:proto_helpers",
        "//packages/planning:utils",
    ],
)
