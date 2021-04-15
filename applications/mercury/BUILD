py_binary(
    name = "mercury",
    srcs = glob(["src/**/*.py"]),
    default_python_version = "PY3",
    main = "src/main.py",
    srcs_version = "PY3",
    deps = [
        ":mercurylib",
    ],
)

py_library(
    name = "mercurylib",
    srcs = glob(["lib/**/*.py"]),
    srcs_version = "PY2AND3",
    deps = [
        "//applications/mercury/proto:mercury_py",
        "@com_github_google_protobuf//:protobuf_python",
    ],
)

py_test(
    name = "mercury_test",
    size = "small",
    srcs = glob(["test/**/*.py"]),
    default_python_version = "PY3",
    main = "test/main.py",
    srcs_version = "PY3",
    deps = [":mercurylib"],
)
