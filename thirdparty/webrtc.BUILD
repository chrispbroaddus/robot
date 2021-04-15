licenses(["notice"])

config_setting(
    name = "opt_mode",
    values = {
        "compilation_mode": "opt",
    },
)

# Prebuilt webrtc library which also includes its dependencies like
# boringssl.
cc_library(
    name = "metrics",
    srcs = select({
        ":opt_mode": [
            "lib/x64/Release/libmetrics_default.a",
        ],
        "//conditions:default": [
            "lib/x64/Debug/libmetrics_default.a",
        ],
    }),
)

cc_library(
    name = "field_trial",
    srcs = select({
        ":opt_mode": [
            "lib/x64/Release/libfield_trial_default.a",
        ],
        "//conditions:default": [
            "lib/x64/Debug/libfield_trial_default.a",
        ],
    }),
)

cc_library(
    name = "webrtc",
    srcs = select({
        ":opt_mode": [
            "lib/x64/Release/libboringssl.a",
            "lib/x64/Release/libcpu_features_linux.a",
            "lib/x64/Release/libsystem_wrappers.a",
            "lib/x64/Release/libwebrtc.a",
        ],
        "//conditions:default": [
            "lib/x64/Debug/libboringssl.a",
            "lib/x64/Debug/libcpu_features_linux.a",
            "lib/x64/Debug/libsystem_wrappers.a",
            "lib/x64/Debug/libwebrtc.a",
        ],
    }),
    hdrs = glob([
        "include/**/*.h",
    ]),
    includes = [
        "include",
        # TODO(mzhao) these includes should be broken into smaller targets
        # like boringssl, protobuf, libyuv
        "include/third_party/boringssl/src/include",
        "include/third_party/libyuv/include",
    ],
    linkopts = [
        "-pthread",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":field_trial",
        ":metrics",
        "@com_github_google_protobuf//:protobuf",
    ],
)
