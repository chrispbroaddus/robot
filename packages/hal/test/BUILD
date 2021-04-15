load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "hal_test",
    srcs = [
        "device_registry_test.cpp",
        "drivers/cameras/dc1394/dc1394_driver_factory_test.cpp",
        "drivers/cameras/dc1394/dc1394_driver_test.cpp",
        "drivers/cameras/flycapture/flycapture_driver_factory_test.cpp",
        "drivers/cameras/flycapture/flycapture_driver_test.cpp",
        "drivers/cameras/unity_simulated/unity_simulated_camera_factory_test.cpp",
        "drivers/cameras/unity_simulated/unity_simulated_camera_test.cpp",
        "drivers/gps/gpsd_device_test.cpp",
        "drivers/imus/unity_simulated/unity_simulated_imu_factory_test.cpp",
        "drivers/imus/unity_simulated/unity_simulated_imu_test.cpp",
        "drivers/network_health/interface_statistics_test.cpp",
        "drivers/network_health/ipv4_header_test.cpp",
        "drivers/network_health/pingv4_test.cpp",
        "drivers/vcu/udp_socket/udp_socket_vcu_driver_factory_test.cpp",
        "drivers/vcu/udp_socket/udp_socket_vcu_driver_test.cpp",
        "drivers/vcu/unity_simulated/unity_simulated_vcu_factory_test.cpp",
        "drivers/vcu/unity_simulated/unity_simulated_vcu_test.cpp",
        "joystick_device_test.cpp",
        "protobuf_io_test.cpp",
    ],
    copts = COPTS,
    tags = ["hardware"],
    deps = [
        "//external:gflags",
        "//packages/hal",
        "@gtest//:main",
    ],
)

cc_test(
    name = "realsense_test",
    srcs = [
        "drivers/cameras/depth/realsense_driver_factory_test.cpp",
        "drivers/cameras/depth/realsense_driver_test.cpp",
    ],
    copts = COPTS,
    tags = ["hardware"],
    deps = [
        "//external:gflags",
        "//packages/hal",
        "@gtest//:main",
    ],
)

cc_test(
    name = "coordinate_frame_id_test",
    srcs = [
        "utilities/coordinate_frame_id_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//external:gflags",
        "//packages/hal",
        "//packages/hal/utilities:coordinate_frame_id_utils",
        "@gtest//:main",
    ],
)
