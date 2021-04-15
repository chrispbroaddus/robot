# https://chromium.googlesource.com/external/webrtc/+/master/webrtc/examples/relayserver/
cc_binary(
    name = "relayserver",
    srcs = [
        "relayserver_main.cc",
    ],
    copts = [
        "-std=c++1y",
        "-pthread",
        "-DWEBRTC_POSIX",
        "-D_GLIBCXX_USE_CXX11_ABI=0",
    ],
    deps = [
        "//external:webrtc",
    ],
)
