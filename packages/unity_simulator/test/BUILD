cc_test(
    name = "vcu_telemetry_receive_test",
    srcs = [
        "vcu_telemetry_comm_test.cpp",
    ],
    data = [
        ":connections.default.json",
        ":simulator_settings.default.json",
        ":system_calibration.default.json",
    ],
    tags = [
        "manual",
    ],
    deps = [
        "//external:gflags",
        "//packages/net",
        "//packages/unity_plugins/connections_reader",
        "//packages/unity_plugins/vcu_telemetry_publisher",
        "@gtest//:main",
    ],
)

cc_test(
    name = "wheel_odometry_test",
    srcs = [
        "wheel_odometry_test.cpp",
    ],
    data = [
        ":connections.default.json",
        ":simulator_settings.default.json",
        ":system_calibration.default.json",
    ],
    tags = [
        "manual",
    ],
    deps = [
        "//external:gflags",
        "//packages/hal/proto:vcu_commands",
        "//packages/hal/proto:vcu_telemetry",
        "//packages/net",
        "//packages/unity_plugins/connections_reader",
        "//packages/unity_plugins/proto:unity_telemetry_envelope",
        "//packages/unity_plugins/vcu_telemetry_publisher",
        "@gtest//:main",
    ],
)

cc_test(
    name = "servo_telemetry_test",
    srcs = [
        "servo_telemetry_test.cpp",
    ],
    data = [
        ":connections.default.json",
        ":simulator_settings.default.json",
        ":system_calibration.default.json",
    ],
    tags = [
        "manual",
    ],
    deps = [
        "//external:gflags",
        "//packages/hal/proto:vcu_commands",
        "//packages/hal/proto:vcu_telemetry",
        "//packages/net",
        "//packages/unity_plugins/connections_reader",
        "//packages/unity_plugins/proto:unity_telemetry_envelope",
        "//packages/unity_plugins/vcu_telemetry_publisher",
        "@gtest//:main",
    ],
)

cc_test(
    name = "hald_compatability_test",
    srcs = [
        "hald_compatability_test.cpp",
    ],
    data = [
        ":connections.default.json",
        ":simulator_settings.default.json",
        ":system_calibration.default.json",
        "//:simulator_configuration",
    ],
    tags = [
        "manual",
    ],
    deps = [
        "//external:gflags",
        "//packages/hald:hald_application",
        "//packages/net",
        "//packages/unity_plugins/connections_reader",
        "@gtest//:main",
    ],
)

cc_test(
    name = "imu_test",
    srcs = [
        "imu_test.cpp",
    ],
    data = [
        ":connections.default.json",
        ":simulator_settings.default.json",
        ":system_calibration.default.json",
    ],
    tags = [
        "manual",
    ],
    deps = [
        "//external:gflags",
        "//packages/hal/proto:imu_sample",
        "//packages/net",
        "//packages/unity_plugins/connections_reader",
        "@gtest//:main",
    ],
)

cc_test(
    name = "gps_test",
    srcs = [
        "gps_test.cpp",
    ],
    data = [
        ":connections.default.json",
        ":simulator_settings.default.json",
        ":system_calibration.default.json",
    ],
    tags = [
        "manual",
    ],
    deps = [
        "//external:gflags",
        "//packages/hal/proto:gps_telemetry",
        "//packages/net",
        "//packages/unity_plugins/connections_reader",
        "@gtest//:main",
    ],
)
