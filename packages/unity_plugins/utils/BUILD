cc_library(
    name = "utils",
    srcs = [
        "src/glog_interop.cpp",
        "src/simulator_time.cpp",
    ],
    hdrs = [
        "include/coordinate_conversion.h",
        "include/gl_error_helper.h",
        "include/vector2.h",
        "include/vector3.h",
        "include/zippy_image_interop.h",
        "include/zippy_interface.h",
    ],
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//external:eigen",
        "//packages/core",
        "//packages/hal",
        "//thirdparty/Sophus",
    ],
)
