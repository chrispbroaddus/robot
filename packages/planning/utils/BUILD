load("//tools:cpp_compile_flags.bzl", "COPTS")

# A python utility to plot serialized trajectory protos.
# Usage:
# $ bazel run :trajectory_plotter /path/to/serialized/protos
py_binary(
    name = "trajectory_plotter",
    srcs = ["trajectory_plotter.py"],
    default_python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        "//packages/planning/proto:trajectory_py",
        "@com_github_google_protobuf//:protobuf_python",
    ],
)

# A stress-test utility to saturate the simulated_vcu with trajectory messages.
# Ensure that both the simulator and hald are running. The utility will ramp the
# publish frequency until timing errors are induced on the simulator.
# Usage:
# $ bazel run :sim_timing_check
cc_binary(
    name = "sim_timing_check",
    srcs = ["sim_timing_check.cpp"],
    copts = COPTS,
    deps = [
        "//packages/planning:planner",
        "//packages/planning:utils",
    ],
)

cc_binary(
    name = "vcu_status_request",
    srcs = ["vcu_status_request.cpp"],
    copts = COPTS,
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/hal",
        "//packages/hal/proto:vcu_requests",
    ],
)
