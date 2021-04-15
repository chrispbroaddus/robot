package(default_visibility = ["//visibility:public"])

# Visualize raw VCU telemetry data. This visualizes:
#   1. VCU-estimated distance travelled
#   2. Estimated instantaneous velocity
# Usage:
# $ bazel run :telemetry_checker -- path/vcu/vcu-2017-08-29T19:25:00.protodat
py_binary(
    name = "telemetry_checker",
    srcs = ["telemetry_checker.py"],
    default_python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        "//packages/hal/proto:vcu_telemetry_py",
        "@com_github_google_protobuf//:protobuf_python",
    ],
)
