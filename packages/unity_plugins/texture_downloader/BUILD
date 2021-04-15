# Texture Downloader
cc_library(
    name = "texture_downloader",
    srcs = [
        "src/Unity/IUnityGraphics.h",
        "src/Unity/IUnityInterface.h",
        "src/camera_download_group.cpp",
        "src/camera_download_group.h",
        "src/frame_data.h",
        "src/image_download_queue.cpp",
        "src/image_download_queue.h",
        "src/platform_base.h",
        "src/renderer_api.cpp",
        "src/renderer_api.h",
        "src/renderer_api_glcore.cpp",
        "src/renderer_api_glcore.h",
        "src/status.h",
        "src/texture_downloader.cpp",
    ],
    defines = [
        "SUPPORT_OPENGL_UNIFIED=1",
        "UNITY_LINUX=1",
        "GLEW_STATIC",
    ],
    linkopts = ["-lGLEW -lGLU -lGL"],
    linkstatic = 0,
    tags = ["sim_plugin"],
    visibility = ["//visibility:public"],
    deps = [
        "//packages/unity_plugins/utils",
    ],
)
