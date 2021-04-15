load("//tools:cpp_compile_flags.bzl", "COPTS")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "logging",
    srcs = glob([
        "src/*.cpp",
        "src/*.h",
    ]),
    hdrs = glob(["include/*.h"]),
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = ["//external:glog"],
)
