cc_test(
    name = "test",
    srcs = [
        "camera_calibration_reader_test.cpp",
    ],
    data = [
        "system_calibration_example.json",
    ],
    tags = ["sim_plugin"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/unity_plugins/camera_calibration_reader",
        "//packages/unity_plugins/utils",
        "@gtest//:main",
    ],
)
