licenses(["permissive"])

# Project    				AprilTags Visual Fiducial System
# License    				BSD
# Current Version 	2016-12-01 (0.9.8)
# https://april.eecs.umich.edu/media/apriltag/apriltag-2016-12-01.tgz

# Note (ian):
# This is currently pre-built and archived at:
#   https://s3-us-west-2.amazonaws.com/zippy.build/dependencies/654e6f3a58944ce97de6de49ac4fae6816da897eb6210b68a67968d00599c881.zip
#
# The reason for this is the dual-license present in the code; "common"
# components have different licensing to those of the rest of the codebase. As
# this package is not available in `apt`, it has been built and is imported as
# a shared library, to conform with licensing restrictions.

cc_library(
    name = "apriltags",
    visibility = ["//visibility:public"],
    srcs = [
      "lib/libapriltag.so"
    ],
    hdrs = glob([
      "include/**/*.h"
      ]),
    linkopts = ["-lpthread"],
    includes = ["include"],
)

cc_binary(
    name = "apriltag_demo",
    srcs = [
      "src/example/apriltag_demo.c"
    ],
    deps = [
      ":apriltags",
    ]
)
