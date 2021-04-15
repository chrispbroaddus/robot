package(default_visibility = ["//visibility:public"])

py_library(
    name = "utils",
    srcs = ["utils.py"],
    srcs_version = "PY2AND3",
    deps = [
        "//packages/hal/proto:camera_sample_py",
        "@com_github_google_protobuf//:protobuf_python",
    ],
)

py_binary(
    name = "point_cloud_plotter",
    srcs = ["point_cloud_plotter.py"],
    default_python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        ":utils",
        "//packages/hal/proto:camera_sample_py",
        "@com_github_google_protobuf//:protobuf_python",
    ],
)

py_binary(
    name = "dataset_trimmer",
    srcs = ["dataset_trimmer.py"],
    default_python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        ":utils",
        "//packages/hal/proto:camera_sample_py",
        "@com_github_google_protobuf//:protobuf_python",
    ],
)

py_binary(
    name = "ground_truth_plotter",
    srcs = ["ground_truth_plotter.py"],
    default_python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        "//packages/unity_plugins/proto:ground_truth_vehicle_pose_py",
        "//packages/unity_plugins/proto:unity_telemetry_envelope_py",
        "@com_github_google_protobuf//:protobuf_python",
    ],
)
