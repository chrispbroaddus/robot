load("@bazel_tools//tools/build_defs/pkg:pkg.bzl", "pkg_tar", "pkg_deb")

cc_library(
    name = "realsense",
    srcs = [
        "src/algo.cpp",
        "src/archive.cpp",
        "src/backend.cpp",
        "src/backend-hid.cpp",
        "src/backend-v4l2.cpp",
        "src/context.cpp",
        "src/device.cpp",
        "src/ds5.cpp",
        "src/ds5-options.cpp",
        "src/ds5-private.cpp",
        "src/ds5-timestamp.cpp",
        "src/error-handling.cpp",
        "src/hw-monitor.cpp",
        "src/image.cpp",
        "src/ivcam-private.cpp",
        "src/log.cpp",
        "src/option.cpp",
        "src/recorder.cpp",
        "src/rs.cpp",
        "src/sql.cpp",
        "src/sr300.cpp",
        "src/subdevice.cpp",
        "src/sync.cpp",
        "src/types.cpp",
        "src/verify.c",
        "third_party/easyloggingpp/src/easylogging++.cc",
        "third_party/sqlite/sqlite3.c",
    ],
    hdrs = [
        "include/librealsense/rs2.h",
        "include/librealsense/rs2.hpp",
        "include/librealsense/rscore2.hpp",
        "include/librealsense/rsutil2.h",
        "include/librealsense/rsutil2.hpp",
        "src/algo.h",
        "src/archive.h",
        "src/backend.h",
        "src/backend-hid.h",
        "src/backend-v4l2.h",
        "src/concurrency.h",
        "src/context.h",
        "src/device.h",
        "src/ds5.h",
        "src/ds5-options.h",
        "src/ds5-private.h",
        "src/ds5-timestamp.h",
        "src/error-handling.h",
        "src/hw-monitor.h",
        "src/image.h",
        "src/ivcam-private.h",
        "src/option.h",
        "src/recorder.h",
        "src/sql.h",
        "src/sr300.h",
        "src/subdevice.h",
        "src/sync.h",
        "src/types.h",
        "third_party/easyloggingpp/src/easylogging++.h",
        "third_party/sqlite/sqlite3.h",
    ],
    copts = [
        "-DRS2_USE_V4L2_BACKEND",
        "-DUNICODE",
        "-fPIC",
        "-g",
        "-D_BSD_SOURCE",
        "-Wno-missing-field-initializers",
        "-Wno-switch",
        "-Wno-multichar",
        "-Wno-unused-variable",
        "-Wno-reorder",
        "-Wno-sign-compare",
        "-Wno-unused-function",
        "-Wno-parentheses",
        "-Wno-return-type",
        "-Wno-unused-value",
        "-Wno-format-security",
        "-Wno-bool-compare",
        "-Wno-unused-but-set-variable",
        "-ftree-vectorize",
        "-I/usr/include/libusb-1.0",
    ],
    includes = ["include"],
    linkopts = [
        "-lusb-1.0",
        "-lpthread",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "imgui",
    srcs = [
        "examples/third_party/imgui/imconfig.h",
        "examples/third_party/imgui/imgui.cpp",
        "examples/third_party/imgui/imgui_draw.cpp",
        "examples/third_party/imgui/imgui_impl_glfw.cpp",
        "examples/third_party/imgui/imgui_internal.h",
    ],
    hdrs = [
        "examples/third_party/imgui/imgui.h",
        "examples/third_party/imgui/imgui_impl_glfw.h",
        "examples/third_party/imgui/stb_rect_pack.h",
        "examples/third_party/imgui/stb_textedit.h",
        "examples/third_party/imgui/stb_truetype.h",
        "examples/third_party/stb_easy_font.h",
        "examples/third_party/stb_image.h",
        "examples/third_party/stb_image_write.h",
    ],
    includes = [
        "examples/third_party/",
        "examples/third_party/imgui",
    ],
    linkopts = [
        "-lglfw",
        "-lGL",
    ],
)

cc_binary(
    name = "cpp-config-ui",
    srcs = [
        "examples/cpp-config-ui.cpp",
        "examples/example.hpp",
    ],
    deps = [
        ":imgui",
        ":realsense",
    ],
)

cc_binary(
    name = "cpp-headless",
    srcs = [
        "examples/cpp-headless.cpp",
        "examples/example.hpp",
    ],
    deps = [
        ":imgui",
        ":realsense",
    ],
)

cc_library(
    name = "tclap",
    hdrs = [
        "examples/third_party/tclap/include/tclap/Arg.h",
        "examples/third_party/tclap/include/tclap/ArgException.h",
        "examples/third_party/tclap/include/tclap/ArgTraits.h",
        "examples/third_party/tclap/include/tclap/CmdLine.h",
        "examples/third_party/tclap/include/tclap/CmdLineInterface.h",
        "examples/third_party/tclap/include/tclap/CmdLineOutput.h",
        "examples/third_party/tclap/include/tclap/Constraint.h",
        "examples/third_party/tclap/include/tclap/DocBookOutput.h",
        "examples/third_party/tclap/include/tclap/HelpVisitor.h",
        "examples/third_party/tclap/include/tclap/IgnoreRestVisitor.h",
        "examples/third_party/tclap/include/tclap/MultiArg.h",
        "examples/third_party/tclap/include/tclap/MultiSwitchArg.h",
        "examples/third_party/tclap/include/tclap/OptionalUnlabeledTracker.h",
        "examples/third_party/tclap/include/tclap/StandardTraits.h",
        "examples/third_party/tclap/include/tclap/StdOutput.h",
        "examples/third_party/tclap/include/tclap/SwitchArg.h",
        "examples/third_party/tclap/include/tclap/UnlabeledMultiArg.h",
        "examples/third_party/tclap/include/tclap/UnlabeledValueArg.h",
        "examples/third_party/tclap/include/tclap/ValueArg.h",
        "examples/third_party/tclap/include/tclap/ValuesConstraint.h",
        "examples/third_party/tclap/include/tclap/VersionVisitor.h",
        "examples/third_party/tclap/include/tclap/Visitor.h",
        "examples/third_party/tclap/include/tclap/XorHandler.h",
        "examples/third_party/tclap/include/tclap/ZshCompletionOutput.h",
    ],
    includes = ["examples/third_party/tclap/include/"],
)

cc_binary(
    name = "cpp-enumerate-devices",
    srcs = [
        "examples/cpp-enumerate-devices.cpp",
        "examples/example.hpp",
    ],
    deps = [
        ":realsense",
        ":tclap",
    ],
)
