cc_library(
    name = "connections_reader",
    srcs = [
        "src/connections_reader.cpp",
        "src/connections_reader_interop.cpp",
    ],
    hdrs = [
        "include/connections_reader.h",
        "include/default_simulator_connections.h",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//applications/mercury/proto:connections",
        "//external:gflags",
        "//external:glog",
        "//packages/unity_plugins/utils",
    ],
)

cc_binary(
    name = "default_connections_writer",
    srcs = [
        "src/default_connections_writer.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "connections_reader",
    ],
)

cc_binary(
    name = "connections_validator",
    srcs = [
        "src/connections_validator.cpp",
    ],
    deps = [
        "connections_reader",
        "//external:glog",
    ],
)
