load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_test(
    name = "imu_prapagator_test",
    srcs = [
        "imu_database_test.cpp",
        "imu_motion_events_test.cpp",
        "imu_orientation_estimator_test.cpp",
        "imu_parameter_estimation_test.cpp",
        "imu_propagator_test.cpp",
        "imu_statistics_test.cpp",
    ],
    copts = COPTS,
    deps = [
        "//packages/imu_propagator",
        "@gtest//:main",
    ],
)
