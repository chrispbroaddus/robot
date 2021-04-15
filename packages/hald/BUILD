load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "haldlib",
    srcs = [
        "src/camera_device_thread.cpp",
        "src/device_thread.cpp",
        "src/gps_device_thread.cpp",
        "src/imu_device_thread.cpp",
        "src/joystick_device_thread.cpp",
        "src/network_health_device_thread.cpp",
        "src/vcu_device_thread.cpp",
    ],
    hdrs = [
        "include/camera_device_thread.h",
        "include/device_thread.h",
        "include/gps_device_thread.h",
        "include/imu_device_thread.h",
        "include/joystick_device_thread.h",
        "include/network_health_device_thread.h",
        "include/vcu_device_thread.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:gflags",
        "//external:zmq",
        "//packages/hal",
        "//packages/hal/proto:ae_roi_sample",
        "//packages/hald:camera_settings_handler",
        "//packages/hald/proto:device_config",
        "//packages/hald/proto:service_list",
        "//packages/net",
    ],
)

cc_library(
    name = "camera_settings_handler",
    srcs = [
        "src/camera_settings_handler.cpp",
    ],
    hdrs = [
        "include/camera_settings_handler.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:zmq",
        "//packages/hal/proto:ae_roi_sample",
        "//packages/net",
        "//packages/teleop/proto:backend_message",
        "//packages/teleop/proto:connection_options",
    ],
)

cc_library(
    name = "hald_application",
    srcs = [":src/hald_application.cpp"],
    hdrs = [":include/hald_application.h"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hald:haldlib",
    ],
)

cc_binary(
    name = "hald",
    srcs = ["src/main.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":hald_application",
        "//external:gflags",
        "//external:glog",
    ],
)
