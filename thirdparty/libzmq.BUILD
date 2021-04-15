licenses(["reciprocal"])

# Project    				ZeroMQ
# License    				LGPL
# Version    				4.2.1
# https://github.com/zeromq/libzmq/releases/download/v4.2.1/zeromq-4.2.1.tar.gz

# Note (avinash):
# This is currently pre-built and archived at:
#   https://s3-us-west-2.amazonaws.com/zippy.build.tmp/dependencies/zeromq_4.2.1-1_amd64.deb
#
# Since the version 4.2.1 of the package is not available in `apt`, it has been built and is imported as
# a shared library, to conform with licensing restrictions.

cc_library(
    name = "zmq",
    visibility = ["//visibility:public"],
    srcs = glob(["lib/libzmq.so*"]),
    hdrs = [
      "include/zmq.h",
      "include/zmq_utils.h",
    ],
)
