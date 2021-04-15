# Build sourcey from git code
cc_library(
    name = "base",
    srcs = glob([
        "src/base/src/*.cpp",
    ]),
    hdrs = glob([
        "src/base/include/scy/*.h",
    ]),
    includes = [
        "src/base/include",
    ],
    linkopts = [
        "-pthread",
    ],
    deps = [
        "//external:sourcey_vendor",
        "//external:sourcey_generated_header",
    ],
)

cc_library(
    name = "av",
    srcs = glob([
        "src/av/src/*.cpp",
    ]),
    hdrs = glob([
        "src/av/include/scy/av/*.h",
    ]),
    includes = [
        "src/av/include",
    ],
    deps = [
        ":base",
        "//external:boringssl",
    ],
)

cc_library(
    name = "crypto",
    srcs = glob([
        "src/crypto/src/*.cpp",
    ]),
    hdrs = glob([
        "src/crypto/include/scy/crypto/*.h",
    ]),
    includes = [
        "src/crypto/include",
    ],
    deps = [
        ":base",
        "//external:boringssl",
    ],
)

cc_library(
    name = "util",
    srcs = glob([
        "src/util/src/*.cpp",
    ]),
    hdrs = glob([
        "src/util/include/scy/util/*.h",
    ]),
    includes = [
        "src/util/include",
    ],
    deps = [
        ":base",
    ],
)

cc_library(
    name = "http",
    srcs = glob([
        "src/http/src/*.cpp",
    ]),
    hdrs = glob([
        "src/http/include/scy/http/*.h",
    ]),
    includes = [
        "src/http/include",
    ],
    deps = [
        ":base",
        ":crypto",
        ":net",
    ],
)

cc_library(
    name = "json",
    srcs = glob([
        "src/json/src/*.cpp",
    ]),
    hdrs = glob([
        "src/json/include/scy/json/*.h",
    ]),
    includes = [
        "src/json/include",
    ],
    deps = [
        ":base",
    ],
)

cc_library(
    name = "net",
    srcs = glob([
        "src/net/src/*.cpp",
    ]),
    hdrs = glob([
        "src/net/include/scy/net/*.h",
    ]),
    includes = [
        "src/net/include",
    ],
    deps = [
        ":base",
        ":crypto",
        ":util",
    ],
)

cc_library(
    name = "socketio",
    srcs = glob([
        "src/socketio/src/*.cpp",
    ]),
    hdrs = glob([
        "src/socketio/include/scy/socketio/*.h",
    ]),
    includes = [
        "src/socketio/include",
    ],
    deps = [
        ":base",
        ":http",
        ":json",
        ":util",
    ],
)

cc_library(
    name = "symple",
    srcs = glob([
        "src/symple/src/*.cpp",
    ]),
    hdrs = glob([
        "src/symple/include/scy/symple/*.h",
    ]),
    includes = [
        "src/symple/include",
    ],
    deps = [
        ":base",
        ":crypto",
        ":json",
        ":net",
        ":socketio",
    ],
)

cc_library(
    name = "webrtc",
    srcs = glob([
        "src/webrtc/src/*.cpp",
    ]),
    hdrs = glob([
        "src/webrtc/include/scy/webrtc/*.h",
    ]),
    copts = [
        "-DWEBRTC_POSIX",
    ],
    includes = [
        "src/webrtc/include",
    ],
    deps = [
        ":base",
        ":json",
        ":net",
        ":util",
        "//external:webrtc",
    ],
)

cc_library(
    name = "sourcey",
    visibility = ["//visibility:public"],
    deps = [
        ":av",
        ":socketio",
        ":symple",
        ":webrtc",
    ],
)

# Samples to make sure sourcey can be properly linked.
cc_binary(
    name = "httpechoserver",
    srcs = [
        "src/http/samples/httpechoserver/httpechoserver.cpp",
        "src/http/samples/httpechoserver/httpechoserver.h",
        "src/http/samples/httpechoserver/libuvhttpserver.h",
    ],
    copts = [
        "-pthread",
        "-DWEBRTC_POSIX",
    ],
    deps = [
        ":sourcey",
    ],
)

cc_binary(
    name = "webrtccapturer",
    srcs = [
        "src/webrtc/samples/webrtccapturer/main.cpp",
        "src/webrtc/samples/webrtccapturer/signaler.cpp",
        "src/webrtc/samples/webrtccapturer/signaler.h",
        "src/webrtc/samples/webrtccapturer/util.h",
    ],
    copts = [
        "-pthread",
        "-DWEBRTC_POSIX",
    ],
    linkopts = [
        "-ldl",
        "-lX11",
        "-lGLU",
    ],
    deps = [
        ":sourcey",
    ],
)
