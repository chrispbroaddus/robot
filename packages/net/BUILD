load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "net",
    srcs = [
        "src/zmq_select.cpp",
    ],
    hdrs = [
        "include/zmq_rep_server.h",
        "include/zmq_req_client.h",
        "include/zmq_select.h",
        "include/zmq_topic_pub.h",
        "include/zmq_topic_sub.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = [
        "//external:cppzmq",
        "//external:glog",
        "//external:protobuf_clib",
        "//external:zmq",
    ],
)
