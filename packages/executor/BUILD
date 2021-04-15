load("//tools:cpp_compile_flags.bzl", "COPTS")

COMMON_COPTS = COPTS

cc_library(
    name = "proto_helpers",
    srcs = ["proto_helpers.cpp"],
    hdrs = ["proto_helpers.h"],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//packages/executor/proto:executor_options",
        "//packages/planning:proto_helpers",
    ],
)

cc_library(
    name = "logging",
    srcs = ["logging.cpp"],
    hdrs = ["logging.h"],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//packages/executor/proto:log",
        "//packages/executor/proto:state",
        "//packages/planning:logging",
        "//packages/planning/proto:state",
    ],
)

cc_library(
    name = "execution",
    srcs =
        [
            "executor.cpp",
        ],
    hdrs =
        [
            "executor.h",
        ],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":logging",
        "//external:glog",
        "//external:wykobi",
        "//packages/core",
        "//packages/core/proto:geometry",
        "//packages/core/proto:timestamp",
        "//packages/estimation",
        "//packages/executor/proto:executor_options",
        "//packages/executor/proto:executor_response",
        "//packages/executor/proto:log",
        "//packages/executor/proto:state",
        "//packages/hal/proto:vcu_commands",
        "//packages/net",
        "//packages/perception",
        "//packages/planning:planner",
        "//packages/planning:utils",
        "//packages/planning/proto:path",
        "//packages/planning/proto:trajectory",
        "//packages/planning/proto:trajectory_options",
        "//packages/planning/proto:trajectory_planner_options",
        "//packages/teleop/proto:backend_message",
    ],
)

#Command - line controller; sends(x, y) coordinates through the planner to either
#the simulator, or robot
cc_binary(
    name = "executor",
    srcs = ["bin/executor.cpp"],
    copts = COMMON_COPTS,
    deps =
        [
            ":execution",
        ],
)

# Echo the contents of a logged file to stdout (similar to the unix `cat` tool
cc_binary(
    name = "cat",
    srcs = ["bin/cat.cpp"],
    copts = COMMON_COPTS,
    deps = [
        ":logging",
        "//external:gflags",
        "//packages/executor/proto:log",
    ],
)
