# Prebuilt libraries and headers that are used by libsourcey
# https://github.com/sourcey/libsourcey
# Licence: LGPL-2.1+
licenses(["notice"])

cc_library(
    name = "vendor",
    srcs = [
        "share/libsourcey/vendor/lib/libhttp_parser.a",
        "share/libsourcey/vendor/lib/liblibuv.a",
        "share/libsourcey/vendor/lib/libminizip.a",
        "share/libsourcey/vendor/lib/libz.a",
    ],
    hdrs = [
        "share/libsourcey/vendor/include/crypt.h",
        "share/libsourcey/vendor/include/http_parser.h",
        "share/libsourcey/vendor/include/ioapi.h",
        "share/libsourcey/vendor/include/json.hpp",
        "share/libsourcey/vendor/include/mztools.h",
        "share/libsourcey/vendor/include/unzip.h",
        "share/libsourcey/vendor/include/uv.h",
        "share/libsourcey/vendor/include/uv-errno.h",
        "share/libsourcey/vendor/include/uv-linux.h",
        "share/libsourcey/vendor/include/uv-threadpool.h",
        "share/libsourcey/vendor/include/uv-unix.h",
        "share/libsourcey/vendor/include/uv-version.h",
        "share/libsourcey/vendor/include/zconf.h",
        "share/libsourcey/vendor/include/zip.h",
        "share/libsourcey/vendor/include/zlib.h",
    ],
    includes = [
        "share/libsourcey/vendor/include",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "sourcey_generated_header",
    hdrs = [
        "linux-k8/libsourcey.h",
    ],
    includes = [
        "linux-k8",
    ],
    visibility = ["//visibility:public"],
)
