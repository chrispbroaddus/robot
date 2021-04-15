load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_binary(
    name = "compare_gt_pose_vs_est_pose_test",
    srcs = [
        "compare_gt_pose_vs_est_pose_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//external:eigen",
        "//external:glog",
        "//packages/calibration/proto:system_calibration",
        "//packages/docking:fiducial_pose_source",
        "//packages/docking/proto:docking_station",
        "//packages/planning:utils",
        "//thirdparty/Sophus",
    ],
)
