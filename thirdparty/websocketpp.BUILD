cc_library(
    name = "websocketpp",
    hdrs = glob(["websocketpp/**/*.hpp"]),
    includes = ["."],
    visibility = ["//visibility:public"],
    defines = [
    	"WEBSOCKETPP_STRICT_MASKING",
    ],
    deps = [
        "@boost//:asio",
    ],
)
