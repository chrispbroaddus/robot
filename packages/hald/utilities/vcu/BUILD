load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_binary(
    name = "dummy_planner",
    srcs = ["dummy_planner_main.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hald:haldlib",
        "//packages/planning:planner",
    ],
)

cc_binary(
    name = "dummy_vcu",
    srcs = ["dummy_vcu_main.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hald:haldlib",
        "//packages/planning:planner",
    ],
)

cc_binary(
    name = "dummy_joystick_planner",
    srcs = ["dummy_joystick_planner.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hald:haldlib",
        "//packages/planning:planner",
    ],
)

cc_binary(
    name = "dummy_telemetry_subscriber",
    srcs = ["dummy_telemetry_subscriber.cpp"],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hald:haldlib",
        "//packages/net",
    ],
)
