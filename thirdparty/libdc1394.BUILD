cc_library(
    name = "dc1394",
    hdrs = glob(["include/dc1394/*.h"]),
    includes= ["include/dc1394"],
    defines = ["_REENTRANT"],
    srcs = glob(["lib/libdc1394*.so*"]),
    visibility = ["//visibility:public"],
)
