load("//tools:cpp_compile_flags.bzl", "COPTS")

COMMON_COPTS = COPTS

cc_library(
    name = "vcu_interop",
    srcs = [
        "vcu_state_machine.h",
        "vcu_trajectory_check.h",
    ],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hal/proto:vcu_commands",
        "@boost//:msm",
    ],
)

cc_library(
    name = "definitions",
    hdrs = [
        "definitions.h",
    ],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "proto_helpers",
    srcs = [
        "proto_helpers.cpp",
        "proto_helpers.hpp",
    ],
    hdrs = ["proto_helpers.h"],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/hal/proto:vcu_commands",
    ],
)

cc_library(
    name = "logging",
    srcs = [
        "logging.cpp",
        "logging.hpp",
    ],
    hdrs = ["logging.h"],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
    deps =
        [
            ":definitions",
            "//external:glog",
            "//packages/core",
            "//packages/core/proto:geometry",
            "//packages/planning/proto:trajectory",
        ],
)

cc_library(
    name = "utils",
    srcs =
        [
            "utils.cpp",
        ],
    hdrs =
        [
            "utils.h",
            "utils.hpp",
        ],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
    deps =
        [
            ":definitions",
            ":logging",
            ":proto_helpers",
            ":vcu_interop",
            "//external:eigen",
            "//external:glog",
            "//external:wykobi",
            "//packages/calibration/proto:coordinate_transformation",
            "//packages/core",
            "//packages/core/proto:geometry",
            "//packages/core/proto:timestamp",
            "//packages/hal/proto:vcu_commands",
            "//packages/net",
            "//packages/planning/proto:path",
            "//packages/planning/proto:trajectory",
            "//thirdparty/Sophus",
        ],
)

cc_library(
    name = "trajectory",
    srcs = ["trajectory.cpp"],
    hdrs = ["trajectory.h"],
    copts = COMMON_COPTS,
    visibility = ["//visibility:public"],
    deps = [
        ":utils",
        "//external:glog",
        "//packages/planning/proto:trajectory",
        "//packages/planning/proto:trajectory_options",
    ],
)

cc_library(
    name = "planner",
    srcs =
        [
            "navigator.cpp",
            "planner.cpp",
        ],
    hdrs =
        [
            "navigator.h",
            "planner.h",
        ],
    copts = COMMON_COPTS,
    data =
        [
            "//:global_configuration",
        ],
    visibility = ["//visibility:public"],
    deps = [
        ":definitions",
        ":logging",
        ":trajectory",
        ":utils",
        "//external:glog",
        "//external:wykobi",
        "//packages/core",
        "//packages/core/proto:geometry",
        "//packages/core/proto:timestamp",
        "//packages/estimation",
        "//packages/executor:proto_helpers",
        "//packages/executor/proto:executor_options",
        "//packages/hal/proto:vcu_commands",
        "//packages/net",
        "//packages/perception",
        "//packages/planning/proto:navigator_options",
        "//packages/planning/proto:path",
        "//packages/planning/proto:state",
        "//packages/planning/proto:trajectory",
        "//packages/planning/proto:trajectory_options",
        "//packages/planning/proto:trajectory_planner_options",
        "//packages/teleop/proto:backend_message",
        "//thirdparty/sbpl:SBPL",
        "@boost//:msm",
    ],
)

#Helper tool to dump default proto options to `.pbtxt` files
cc_binary(
    name = "write_options",
    srcs = ["bin/write_options.cpp"],
    copts = COMMON_COPTS,
    deps =
        [
            ":planner",
        ],
)

# Usage:
# $ bazel run :navigator -- -x 1.0 -y 1.0
cc_binary(
    name = "navigator",
    srcs = ["bin/navigator.cpp"],
    copts = COMMON_COPTS,
    deps =
        [
            ":planner",
        ],
)

# Visualize planning paths
# Usage:
# $ bazel run :visualize_planning
cc_binary(
    name = "visualize_planning",
    srcs = ["bin/visualize_planning.cpp"],
    copts = ["-DHAVE_OPENGL"] + COMMON_COPTS,
    data = ["//packages/perception:bin_shaders"],
    deps = [
        "//external:SDL",
        "//external:glog",
        "//packages/estimation",
        "//packages/estimation:visualization",
        "//packages/executor/proto:executor_options",
        "//packages/perception",
        "//packages/perception:trackball",
        "//packages/perception:visualization",
        "//packages/planning:planner",
        "//packages/planning:utils",
    ],
)

# Replay paths
# Usage:
# $ bazel run :navigation_replay -- -log_file path/to/zmqcomms.log
cc_binary(
    name = "navigation_replay",
    srcs = ["bin/navigation_replay.cpp"],
    copts = COMMON_COPTS,
    deps = [
        ":logging",
        ":planner",
        "//packages/net",
    ],
)

# Stress-test simulator
# Usage:
# $ bazel run :exercise_simulator -- -velocity 1.0 -planner_frequency 10
cc_binary(
    name = "exercise_simulator",
    srcs = ["bin/exercise_simulator.cpp"],
    copts = COMMON_COPTS,
    deps = [
        ":planner",
        "//packages/executor/proto:executor_options",
        "//packages/net",
    ],
)
