load("@io_bazel_rules_python//python:python.bzl", "py_binary", "py_library", "py_test")
load("@pip_deps//:requirements.bzl", "requirement")

py_binary(
    name = "evaluator_queue",
    srcs = ["app.py"],
    main = "app.py",
    srcs_version = "PY2",
    deps = [requirement("Flask")],
)

py_binary(
    name = "evaluator_loop",
    srcs = ["loop.py"],
    main = "loop.py",
    srcs_version = "PY2",
)
