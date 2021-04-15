#
# gtest
#
new_http_archive(
    name = "gtest",
    url = "https://github.com/google/googletest/archive/release-1.7.0.zip",
    build_file = "thirdparty/gtest.BUILD",
    strip_prefix = "googletest-release-1.7.0",
    sha256 = "b58cb7547a28b2c718d1e38aee18a3659c9e3ff52440297e965f5edffe34b6d0",
)

bind(
    name = "glog",
    actual = "//thirdparty/glog",
)

bind(
    name = "gflags",
    actual = "//thirdparty/gflags",
)

#
# Eigen
#
new_http_archive(
    name = "eigen_repo",
    url = "http://bitbucket.org/eigen/eigen/get/3.3.3.zip",
    build_file = "thirdparty/eigen.BUILD",
    strip_prefix = "eigen-eigen-67e894c6cd8f",
    sha256 = "47cc7d7f229d7ea3ee6adb2015b23bff81b5bc66e292e44fb7dee34a9230556e",
)

bind(
    name = "eigen",
    actual = "@eigen_repo//:eigen"
)

#
# DC1394
#
new_local_repository(
    name = "libdc1394",
    build_file = "thirdparty/libdc1394.BUILD",
    path = "/usr/local",
)

#
# Tensorflow
#
new_local_repository(
    name = "libtensorflow",
    build_file = "thirdparty/libtensorflow.BUILD",
    path = "/usr/local",
)

#
# Flycapture
#
new_local_repository(
    name = "libflycapture",
    build_file = "thirdparty/flycapture.BUILD",
    path = "/usr",
)

bind(
    name = "flycapture",
    actual = "@libflycapture//:libflycapture",
)

#
# Wykobi
# A simple slim 2d/3d geometry library:
# http://www.wykobi.com/
new_http_archive(
    name = "wykobi_repo",
    url = "https://s3-us-west-2.amazonaws.com/zippy.build.tmp/dependencies/e42f5a44a84cbb604a3b6f56f832463f16b553f613d144ef605a88392082462b.zip",
    build_file = "thirdparty/wykobi.BUILD",
)

bind(
    name = "wykobi",
    actual = "@wykobi_repo//:wykobi",
)

#
# SDL
#
new_http_archive(
    name = "SDL_repo",
    url = "https://s3-us-west-2.amazonaws.com/zippy.build.tmp/dependencies/c91bf3158dfaab53321b8820754e37f16e81f9dd45f44cce1b7dc90f5ccfb216.zip",
    strip_prefix = "SDL2",
    build_file = "thirdparty/sdl.BUILD",
    sha256 = "c91bf3158dfaab53321b8820754e37f16e81f9dd45f44cce1b7dc90f5ccfb216",
)

bind(
  name = "SDL",
  actual = "@SDL_repo//:SDL"
)

#
# Protobuf rules
#
load("//protobuf_rules/protobuf:rules.bzl", "proto_repositories")
proto_repositories()

load("//protobuf_rules/cpp:rules.bzl", "cpp_proto_repositories")
cpp_proto_repositories()

load("//protobuf_rules/python:rules.bzl", "py_proto_repositories")
py_proto_repositories()

#
# ZMQ
#
new_local_repository(
    name = "libzmq",
    build_file = "thirdparty/libzmq.BUILD",
    path = "/usr/local",
)

bind(
  name = "zmq",
  actual = "@libzmq//:zmq"
)

#
# CPPZMQ
#
new_http_archive(
    name = "cppzmq_repo",
    url = "https://github.com/zeromq/cppzmq/archive/v4.2.1.tar.gz",
    strip_prefix = "cppzmq-4.2.1",
    build_file = "thirdparty/cppzmq.BUILD",
    sha256 = "11c699001659336c7d46779f714f3e9d15d63343cd2ae7c1905e4bf58907cef9",
)

bind(
  name = "cppzmq",
  actual = "@cppzmq_repo//:cppzmq"
)

#
# OpenCV-3.2
#
new_local_repository(
    name = "opencv_repo",
    path = "/usr/local",
    build_file = "thirdparty/opencv.BUILD",
)

bind(
    name = "opencv",
    actual = "@opencv_repo//:opencv",
)

# Require six dependency for google proto
new_http_archive(
    name = "six_archive",
    build_file = "thirdparty/six.BUILD",
    sha256 = "105f8d68616f8248e24bf0e9372ef04d3cc10104f1980f54d57b2ce73a5ad56a",
    url = "https://pypi.python.org/packages/source/s/six/six-1.10.0.tar.gz#md5=34eed507548117b2ab523ab14b2f8b55",
)

bind(
    name = "six",
    actual = "@six_archive//:six",
)

#
# Websocketpp
#
new_http_archive(
    name = "websocketpp",
    urls = [
        "http://bazel-mirror.storage.googleapis.com/github.com/zaphoyd/websocketpp/archive/0.7.0.zip",
        "https://github.com/zaphoyd/websocketpp/archive/0.7.0.zip",
    ],
    sha256 = "547abdfc372a2e11a5cdde5cca3fb5e66f41d941534430180d98e9f3561da055",
    strip_prefix = "websocketpp-0.7.0",
    build_file = "thirdparty/websocketpp.BUILD",
)

#
# zlib
#
new_http_archive(
    name = "zlib_archive",
    urls = [
        "http://bazel-mirror.storage.googleapis.com/zlib.net/zlib-1.2.8.tar.gz",
        "http://zlib.net/fossils/zlib-1.2.8.tar.gz",
    ],
    sha256 = "36658cb768a54c1d4dec43c3116c27ed893e88b02ecfcb44f2166f9c0b7f2a0d",
    strip_prefix = "zlib-1.2.8",
    build_file = "thirdparty/zlib.BUILD",
)

bind(
    name = "zlib",
    actual = "@zlib_archive//:zlib",
)

#
# libpng
#
new_http_archive(
    name = "png_archive",
    urls = [
        "http://bazel-mirror.storage.googleapis.com/github.com/glennrp/libpng/archive/v1.2.53.zip",
        "https://github.com/glennrp/libpng/archive/v1.2.53.zip",
    ],
    sha256 = "c35bcc6387495ee6e757507a68ba036d38ad05b415c2553b3debe2a57647a692",
    strip_prefix = "libpng-1.2.53",
    build_file = "thirdparty/png.BUILD",
)

bind(
    name = "png",
    actual = "@png_archive//:png",
)

#
# libjpeg
#
new_http_archive(
    name = "jpeg_archive",
    urls = [
        "http://bazel-mirror.storage.googleapis.com/github.com/libjpeg-turbo/libjpeg-turbo/archive/1.5.1.tar.gz",
        "https://github.com/libjpeg-turbo/libjpeg-turbo/archive/1.5.1.tar.gz",
    ],
    sha256 = "c15a9607892113946379ccea3ca8b85018301b200754f209453ab21674268e77",
    strip_prefix = "libjpeg-turbo-1.5.1",
    build_file = "thirdparty/jpeg.BUILD",
)

bind(
    name = "jpeg",
    actual = "@jpeg_archive//:jpeg",
)

#
# NASM
#
new_http_archive(
    name = "nasm",
    urls = [
      "http://bazel-mirror.storage.googleapis.com/www.nasm.us/pub/nasm/releasebuilds/2.12.02/nasm-2.12.02.tar.bz2",
      "http://pkgs.fedoraproject.org/repo/pkgs/nasm/nasm-2.12.02.tar.bz2/d15843c3fb7db39af80571ee27ec6fad/nasm-2.12.02.tar.bz2",
    ],
    sha256 = "00b0891c678c065446ca59bcee64719d0096d54d6886e6e472aeee2e170ae324",
    strip_prefix = "nasm-2.12.02",
    build_file = "thirdparty/nasm.BUILD",
)

#
# Apriltags
#
new_http_archive(
    name = "apriltags_archive",
    urls = [
        "https://s3-us-west-2.amazonaws.com/zippy.build.tmp/dependencies/654e6f3a58944ce97de6de49ac4fae6816da897eb6210b68a67968d00599c881.zip"
    ],
    sha256 = "654e6f3a58944ce97de6de49ac4fae6816da897eb6210b68a67968d00599c881",
    strip_prefix = "apriltags",
    build_file = "thirdparty/apriltags.BUILD",
)

bind(
    name = "apriltags",
    actual = "@apriltags_archive//:apriltags",
)

bind(
    name = "python_headers",
    actual = "@com_github_google_protobuf//util/python:python_headers",
)

#
# gpsd
#
new_local_repository(
    name = "libgps",
    build_file = "thirdparty/gpsd.BUILD",
    path = "/usr",
)

#
# boost 1.58.0 build with bazel
#
http_archive(
     name = "boost",
     sha256 = "24801159ab1614c9d4a5ad82a94dc2dfbc4bd95b67ade3dccfd35737ba90032f",
     url = "https://github.com/bazelment/boost/archive/boost-1.58.0-2.zip",
     strip_prefix = "boost-boost-1.58.0-2"
)

new_local_repository(
    name = "webrtc_prebuilt",
    build_file = "thirdparty/webrtc.BUILD",
    path = "thirdparty/webrtc/prebuilt",
)

bind(
    name = "webrtc",
    actual = "@webrtc_prebuilt//:webrtc",
)

# TODO(mzhaom) use boringssl from source code and remove the bundle
# boringssl lib from webrtc.
bind(
    name = "boringssl",
    actual = "@webrtc_prebuilt//:webrtc",
)

#
# Nanopb and deps
#
git_repository(
    name = "fi_kapsi_jpa_nanopb",
    commit = "5abfdfdae6f59651edd59cadb9ad26eb19151721",
    remote = "https://github.com/BrendanDrewZippy/nanopb.git",
)

http_archive(
    name = "com_google_protobuf",
    sha256 = "ab499973b61293571964b87f7615d262384e82ee969642c07a0b26013059d712",
    strip_prefix = "protobuf-3.3.0",
    url = "https://github.com/google/protobuf/archive/v3.3.0.zip",
)

#
# STM32 Toolchain, HAL, and rules
#
git_repository(
    name = "stm32",
    remote = "https://github.com/pedrokiefer/rules_stm32.git",
    commit = "0ce7bfc2f878f27c70517f719665e80ec7b4b83d"
)

load("@stm32//stm32f4:rules.bzl", "arm_none_repository")
arm_none_repository("linux")

# sourcey and related
bind(
    name = "sourcey_vendor",
    actual = "//thirdparty/sourcey:vendor",
)

bind(
    name = "sourcey_generated_header",
    actual = "//thirdparty/sourcey:sourcey_generated_header",
)

new_git_repository(
    name = "sourcey_repo",
    remote = "https://github.com/sourcey/libsourcey.git",
    commit = "42712495e211721832656d62421721965dbaa141",
    build_file = "thirdparty/sourcey/sourcey.BUILD"
)

bind(
    name = "sourcey",
    actual = "@sourcey_repo//:sourcey",
)

git_repository(
    name = "io_bazel_rules_python",
    remote = "https://github.com/bazelbuild/rules_python.git",
    commit = "d6fbb0fb2a5c8e318dd4de5104dc41358cefaa90",
)

# Only needed for PIP support:
load("@io_bazel_rules_python//python:pip.bzl", "pip_repositories")

pip_repositories()

load("@io_bazel_rules_python//python:pip.bzl", "pip_import")

# This rule translates the specified requirements.txt into
# @pip_deps//:requirements.bzl, which itself exposes a pip_install method.
pip_import(
   name = "pip_deps",
   requirements = "//applications/evaluator_runner:requirements.txt",
)

# Load the pip_install symbol for pip_deps, and create the dependencies'
# repositories.
load("@pip_deps//:requirements.bzl", "pip_install")
pip_install()

new_http_archive(
    name = "ceres_solver_repo",
    url = "http://ceres-solver.org/ceres-solver-1.13.0.tar.gz",
    build_file = "thirdparty/ceres-solver.BUILD",
    strip_prefix = "ceres-solver-1.13.0",
    sha256 = "1df490a197634d3aab0a65687decd362912869c85a61090ff66f073c967a7dcd",
)

bind(
    name = "ceres-solver",
    actual = "@ceres_solver_repo//:ceres-solver"
)
