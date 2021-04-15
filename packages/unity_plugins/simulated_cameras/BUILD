cc_library(
    name = "simulated_cameras",
    srcs = [
        "src/camera_publisher.cpp",
        "src/camera_publisher_group.cpp",
        "src/simulator_command_server.cpp",
        "src/simulator_network_manager.cpp",
        "src/simulator_network_manager_interop.cpp",
        "src/simulator_stats_publisher.cpp",
    ],
    hdrs = [
        "include/camera_output_addresses.h",
        "include/camera_publisher.h",
        "include/camera_publisher_group.h",
        "include/simulator_command_server.h",
        "include/simulator_network_interop.h",
        "include/simulator_network_manager.h",
        "include/simulator_publisher_topics.h",
        "include/simulator_stats.h",
        "include/simulator_stats_publisher.h",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hal",
        "//packages/hal/proto:camera_sample",
        "//packages/hal/proto:simulator_commands",
        "//packages/hal/proto:simulator_stats",
        "//packages/net",
        "//packages/unity_plugins/utils",
    ],
)

cc_binary(
    name = "test_camera_subscriber",
    srcs = [
        "src/test_camera_subscriber.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "simulated_cameras",
        "//external:glog",
        "//packages/hal",
        "//packages/hal/proto:camera_sample",
        "//packages/net",
        "//packages/unity_plugins/image_saver",
    ],
)

cc_binary(
    name = "test_simulator_command_client",
    srcs = [
        "src/test_simulator_command_client.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "//external:glog",
        "//packages/hal",
        "//packages/hal/proto:simulator_commands",
        "//packages/net",
    ],
)

cc_binary(
    name = "test_stats_subscriber",
    srcs = [
        "src/test_stats_subscriber.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "simulated_cameras",
        "//external:glog",
        "//packages/hal",
        "//packages/hal/proto:simulator_stats",
        "//packages/net",
    ],
)
