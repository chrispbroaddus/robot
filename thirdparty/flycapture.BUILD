licenses(["reciprocal"])

# Project    				Flycapture SDK
# https://www.ptgrey.com/support/downloads/10767/
#
# Note (avinash):
# The same version of the tar file in the link above has also been copied to S3 as backup.
# The ci system downloads the file from the S3 link given below
#   https://s3-us-west-2.amazonaws.com/zippy.build.tmp/dependencies/flycapture2-2.11.3.121-amd64-pkg.tgz

cc_library(
    name = "libflycapture",
    visibility = ["//visibility:public"],
    srcs = glob(["lib/libflycapture.so*"]),
    hdrs = [
      "include/flycapture/AVIRecorder.h",
      "include/flycapture/BusManager.h",
      "include/flycapture/Camera.h",
      "include/flycapture/CameraBase.h",
      "include/flycapture/Error.h",
      "include/flycapture/FlyCapture2.h",
      "include/flycapture/FlyCapture2Defs.h",
      "include/flycapture/FlyCapture2Platform.h",
      "include/flycapture/GigECamera.h",
      "include/flycapture/Image.h",
      "include/flycapture/ImageStatistics.h",
      "include/flycapture/TopologyNode.h",
      "include/flycapture/Utilities.h",
    ],
    includes= ["include/flycapture"],
)
