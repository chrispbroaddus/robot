cc_library(
    name = "vehicle_command_to_motor_control",
    hdrs = [
        "controller.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
    ],
)

cc_binary(
    name = "motor_control_example",
    srcs = [
        "main.cpp",
    ],
    deps = [
        "vehicle_command_to_motor_control",
    ],
)
