load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "net_test",
    srcs = [
        "zmq_select_test.cpp",
        "zmq_topic_pubsub_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//external:gflags",
        "//external:glog",
        "//packages/core/test:common",
        "//packages/hal/proto:camera_sample",
        "//packages/hald/proto:service_list",
        "//packages/net",
        "@gtest//:main",
    ],
)
