cc_library(
    name = "simulator_settings_reader",
    srcs = [
        "src/simulator_settings_reader.cpp",
        "src/simulator_settings_reader_interop.cpp",
    ],
    hdrs = [
        "include/default_simulator_settings.h",
        "include/simulator_settings_reader.h",
        "include/simulator_settings_reader_interop.h",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/unity_plugins/utils",
        "//packages/unity_simulator/proto:simulator_settings",
    ],
)

cc_binary(
    name = "default_settings_writer",
    srcs = [
        "src/default_settings_writer.cpp",
    ],
    tags = ["sim_plugin"],
    deps = [
        "simulator_settings_reader",
    ],
)

cc_binary(
    name = "simulator_settings_validator",
    srcs = [
        "src/simulator_settings_validator.cpp",
    ],
    deps = [
        "simulator_settings_reader",
        "//external:glog",
    ],
)
