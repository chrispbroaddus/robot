# Image Saver
cc_library(
    name = "image_saver",
    srcs = [
        "src/image_saver.cpp",
        "src/stb/stb.h",
        "src/stb/stb_image_write.h",
    ],
    hdrs = [
        "include/image_saver.h",
    ],
    defines = ["UNITY_LINUX=1"],
    linkstatic = 0,
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//external:glog",
        "//packages/unity_plugins/utils",
    ],
)
