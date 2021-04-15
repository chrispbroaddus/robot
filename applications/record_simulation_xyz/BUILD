cc_binary(
    name = "record_simulation_xyz",
    srcs = glob([
        "src/*.cpp",
        "src/*.h",
    ]),
    visibility = ["//visibility:public"],
    deps = [
        "//packages/core",
        "//packages/estimation/proto:state",
        "//packages/hal",
        "//packages/hal/proto:camera_sample",
        "//packages/net",
        "//packages/serialization",
        "//packages/unity_plugins/proto:unity_telemetry_envelope",
        "//packages/unity_simulator/proto:simulator_settings",
        "//thirdparty/gflags",
        "//thirdparty/glog",
    ],
)
