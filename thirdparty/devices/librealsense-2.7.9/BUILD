load("@bazel_tools//tools/build_defs/pkg:pkg.bzl", "pkg_tar", "pkg_deb")

cc_library(
    name = "realsense",
    srcs = [
        "third-party/easyloggingpp/src/easylogging++.cc",
        "third-party/sqlite/sqlite3.c",
    ] + glob([
        "src/**/*.cpp",
    ]),
    hdrs = glob([
        "src/**/*.h",
        "src/**/*.hpp",
        "include/librealsense2/**/*.h",
        "include/librealsense2/**/*.hpp",
        "third-party/easyloggingpp/src/easylogging++.h",
        "third-party/sqlite/sqlite3.h",
    ]),
    copts = [
        "-DRS2_USE_V4L2_BACKEND",
        "-DUNICODE",
        "-DHWM_OVER_XU",
        "-DELPP_THREAD_SAFE",
        "-DELPP_NO_DEFAULT_LOG_FILE",
        "-fPIC",
        "-Wno-unused-variable",
        "-Wno-unused-value",
        "-Wno-reorder",
        "-Wno-bool-compare",
        "-Wno-multichar",
        "-Wno-sequence-point",
        "-Wno-unknown-pragmas",
        "-Wno-unused-but-set-variable",
        "-Wno-overflow",
        "-Wno-sign-compare",
        "-ftree-vectorize",
        "-I/usr/include/libusb-1.0",
    ],
    includes = [
        "include/",
        "src/",
    ],
    linkopts = [
        "-lusb-1.0",
        "-lpthread",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":tclap",
    ],
)

cc_inc_library(
    name = "tclap",
    hdrs = glob(["third-party/tclap/**/*.h"]),
)
