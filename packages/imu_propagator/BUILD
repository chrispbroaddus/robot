load("//tools:cpp_compile_flags.bzl", "COPTS")

cc_library(
    name = "imu_propagator",
    srcs = [
        "src/imu_parameter_estimation.cpp",
        "src/imu_propagator.cpp",
    ],
    hdrs = [
        "include/imu_database.h",
        "include/imu_database_details.h",
        "include/imu_motion_events.h",
        "include/imu_orientation_estimator.h",
        "include/imu_parameter_estimation.h",
        "include/imu_propagator.h",
        "include/imu_propagator_details.h",
        "include/imu_propagator_ode.h",
        "include/imu_propagator_utils.h",
        "include/imu_statistics.h",
        "include/runge_kutta_integrator.h",
    ],
    copts = COPTS,
    visibility = ["//visibility:public"],
    deps = ["//external:eigen"],
)
